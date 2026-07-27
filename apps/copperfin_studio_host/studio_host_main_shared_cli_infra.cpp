// Copyright © 2026 Richard M. Hamilton. All rights reserved.
// Licensed under the Project Copperfin Source-Available License or
// Commercial License. See LICENSE.md in the repository root.

#include "studio_host_main_support.h"
#include "copperfin/licensing/license_status_display.h"

namespace cf_studio_host_main_detail {
const copperfin::localization::LocalizedCatalog* g_active_catalog = nullptr;
std::string g_executable_path;

copperfin::localization::LocalizedCatalog load_localization(std::string_view executable_path) {
    const std::filesystem::path locale_root = copperfin::localization::resolve_catalog_root(
        executable_path.empty()
            ? std::filesystem::path{}
            : copperfin::platform::path_from_utf8_string(executable_path));
    return copperfin::localization::load_catalogs(
        locale_root,
        copperfin::localization::select_locale());
}

std::string localized_message_or_default(
    std::string_view key,
    std::string_view fallback) {
    if (g_active_catalog == nullptr) {
        return std::string(fallback);
    }
    const std::string translated = g_active_catalog->translate(key);
    return translated == key ? std::string(fallback) : translated;
}

std::string studio_error_prefix() {
    return localized_message_or_default("StudioHost.Prefix.Error", "error: ");
}

std::string studio_warning_prefix() {
    return localized_message_or_default("StudioHost.Prefix.Warning", "warning: ");
}

void print_primary_usage_line(
    const copperfin::localization::LocalizedCatalog& catalog,
    std::string_view usage_template) {
    std::cout << catalog.translate(
        "StudioHost.Usage.Primary",
        {{"usageTemplate", std::string(usage_template)}}) << "\n";
}

void print_alternate_usage_line(
    const copperfin::localization::LocalizedCatalog& catalog,
    std::string_view usage_template) {
    std::cout << catalog.translate(
        "StudioHost.Usage.Alternate",
        {{"usageTemplate", std::string(usage_template)}}) << "\n";
}

void print_object_usage_line(
    const copperfin::localization::LocalizedCatalog& catalog,
    std::string_view object_name,
    std::string_view usage_template) {
    std::cout << catalog.translate(
        "StudioHost.Usage.ObjectEntry",
        {
            {"objectName", std::string(object_name)},
            {"usageTemplate", std::string(usage_template)}
        }) << "\n";
}

void print_localized_object_usage_line(
    const copperfin::localization::LocalizedCatalog& catalog,
    std::string_view object_name_key,
    std::string_view usage_template) {
    print_object_usage_line(catalog, catalog.translate(object_name_key), usage_template);
}

void print_selection_context_tokens_line(
    const copperfin::localization::LocalizedCatalog& catalog,
    std::string_view selection_context_tokens) {
    std::cout << catalog.translate(
        "StudioHost.Usage.SelectionContextTokens",
        {{"selectionContextTokens", std::string(selection_context_tokens)}}) << "\n";
}

void print_usage(const copperfin::localization::LocalizedCatalog& catalog) {
    print_alternate_usage_line(catalog,
        "copperfin_studio_host --rushmore-explain [--json] [--rushmore-plan-kind <table_scan|index_seek|index_range_scan>] [--rushmore-cursor <id>] [--rushmore-expression <text>] [--rushmore-index-name <name>] [--rushmore-indexable-predicate <text>] [--rushmore-residual-predicate <text>] [--rushmore-fallback-reason <name>]");
    print_primary_usage_line(catalog, "copperfin_studio_host --path <asset> [--from-vs] [--read-only] [--json] [--selection-context <token>] [--delete-object|--restore-object|--duplicate-object|--rename-object|--reparent-object|--reorder-object|--group-object|--align-object|--resize-object|--distribute-object|--snap-object|--nudge-object|--tab-order-object|--tab-stop-object|--visibility-object|--enabled-object|--read-only-object|--locked-object|--caption-object|--picture-object|--down-picture-object|--disabled-picture-object|--ole-drag-picture-object|--mouse-icon-object|--drag-icon-object|--drag-mode-object|--ole-drag-mode-object|--tooltip-text-object|--status-bar-text-object|--control-source-object|--input-mask-object|--format-object|--row-source-object|--row-source-type-object|--bound-column-object|--column-count-object|--style-object|--list-index-object|--left-column-object|--auto-center-object|--auto-size-object|--auto-release-object|--continuous-scroll-object|--dockable-object|--clip-controls-object|--sparse-object|--lock-screen-object|--allow-cell-selection-object|--delete-mark-object|--record-mark-object|--split-bar-object|--highlight-row-object|--panel-link-object|--allow-header-sizing-object|--allow-row-sizing-object|--resizable-object|--add-line-feeds-object|--always-on-top-object|--always-on-bottom-object|--ungroup-object] [--set-property|--clear-property|--rename-property --record <n> --object-name <name> --unique-id <id> --property-name <name> --property-value <value> --new-property-name <name>] [--new-object-name <name>] [--new-name <name>] [--new-unique-id <id>] [--parent-name <name>] [--parent-unique-id <id>] [--clear-parent] [--placement <front|back|before|after>] [--target-object-name <name>] [--target-unique-id <id>] [--group-child-object-name <name>] [--group-child-unique-id <id>] [--field-value <name=value>] [--alignment-mode <mode>] [--resize-mode <width|height|size>] [--distribution-mode <horizontal|vertical>] [--snap-mode <horizontal|vertical|both>] [--nudge-mode <horizontal|vertical|both>] [--grid-width <n>] [--grid-height <n>] [--delta-hpos <n>] [--delta-vpos <n>] [--starting-tab-index <n>] [--tab-stop <true|false>] [--visible <true|false>] [--enabled <true|false>] [--object-read-only <true|false>] [--locked <true|false>] [--caption <value>] [--picture <value>] [--down-picture <value>] [--disabled-picture <value>] [--ole-drag-picture <value>] [--mouse-icon <value>] [--drag-icon <value>] [--drag-mode <n>] [--ole-drag-mode <n>] [--tooltip-text <value>] [--status-bar-text <value>] [--control-source <value>] [--input-mask <value>] [--format <value>] [--row-source <value>] [--row-source-type <n>] [--bound-column <n>] [--column-count <n>] [--style <n>] [--list-index <n>] [--left-column <n>] [--auto-center <true|false>] [--auto-size <true|false>] [--auto-release <true|false>] [--continuous-scroll <true|false>] [--dockable <true|false>] [--clip-controls <true|false>] [--sparse <true|false>] [--lock-screen <true|false>] [--allow-cell-selection <true|false>] [--delete-mark <true|false>] [--record-mark <true|false>] [--split-bar <true|false>] [--highlight-row <true|false>] [--panel-link <true|false>] [--allow-header-sizing <true|false>] [--allow-row-sizing <true|false>] [--resizable <true|false>] [--add-line-feeds <true|false>] [--always-on-top <true|false>] [--always-on-bottom <true|false>] [--anchor-object-name <name>] [--anchor-unique-id <id>] [--align-target-object-name <name>] [--align-target-unique-id <id>] [--resize-target-object-name <name>] [--resize-target-unique-id <id>] [--distribute-target-object-name <name>] [--distribute-target-unique-id <id>] [--snap-target-object-name <name>] [--snap-target-unique-id <id>] [--nudge-target-object-name <name>] [--nudge-target-unique-id <id>] [--tab-order-target-object-name <name>] [--tab-order-target-unique-id <id>] [--tab-stop-target-object-name <name>] [--tab-stop-target-unique-id <id>] [--visibility-target-object-name <name>] [--visibility-target-unique-id <id>] [--enabled-target-object-name <name>] [--enabled-target-unique-id <id>] [--read-only-target-object-name <name>] [--read-only-target-unique-id <id>] [--locked-target-object-name <name>] [--locked-target-unique-id <id>] [--caption-target-object-name <name>] [--caption-target-unique-id <id>] [--picture-target-object-name <name>] [--picture-target-unique-id <id>] [--down-picture-target-object-name <name>] [--down-picture-target-unique-id <id>] [--disabled-picture-target-object-name <name>] [--disabled-picture-target-unique-id <id>] [--ole-drag-picture-target-object-name <name>] [--ole-drag-picture-target-unique-id <id>] [--mouse-icon-target-object-name <name>] [--mouse-icon-target-unique-id <id>] [--drag-icon-target-object-name <name>] [--drag-icon-target-unique-id <id>] [--drag-mode-target-object-name <name>] [--drag-mode-target-unique-id <id>] [--ole-drag-mode-target-object-name <name>] [--ole-drag-mode-target-unique-id <id>] [--tooltip-text-target-object-name <name>] [--tooltip-text-target-unique-id <id>] [--status-bar-text-target-object-name <name>] [--status-bar-text-target-unique-id <id>] [--control-source-target-object-name <name>] [--control-source-target-unique-id <id>] [--input-mask-target-object-name <name>] [--input-mask-target-unique-id <id>] [--format-target-object-name <name>] [--format-target-unique-id <id>] [--row-source-target-object-name <name>] [--row-source-target-unique-id <id>] [--row-source-type-target-object-name <name>] [--row-source-type-target-unique-id <id>] [--bound-column-target-object-name <name>] [--bound-column-target-unique-id <id>] [--column-count-target-object-name <name>] [--column-count-target-unique-id <id>] [--style-target-object-name <name>] [--style-target-unique-id <id>] [--list-index-target-object-name <name>] [--list-index-target-unique-id <id>] [--left-column-target-object-name <name>] [--left-column-target-unique-id <id>] [--auto-center-target-object-name <name>] [--auto-center-target-unique-id <id>] [--auto-size-target-object-name <name>] [--auto-size-target-unique-id <id>] [--auto-release-target-object-name <name>] [--auto-release-target-unique-id <id>] [--continuous-scroll-target-object-name <name>] [--continuous-scroll-target-unique-id <id>] [--dockable-target-object-name <name>] [--dockable-target-unique-id <id>] [--clip-controls-target-object-name <name>] [--clip-controls-target-unique-id <id>] [--sparse-target-object-name <name>] [--sparse-target-unique-id <id>] [--lock-screen-target-object-name <name>] [--lock-screen-target-unique-id <id>] [--allow-cell-selection-target-object-name <name>] [--allow-cell-selection-target-unique-id <id>] [--delete-mark-target-object-name <name>] [--delete-mark-target-unique-id <id>] [--record-mark-target-object-name <name>] [--record-mark-target-unique-id <id>] [--split-bar-target-object-name <name>] [--split-bar-target-unique-id <id>] [--highlight-row-target-object-name <name>] [--highlight-row-target-unique-id <id>] [--panel-link-target-object-name <name>] [--panel-link-target-unique-id <id>] [--allow-header-sizing-target-object-name <name>] [--allow-header-sizing-target-unique-id <id>] [--allow-row-sizing-target-object-name <name>] [--allow-row-sizing-target-unique-id <id>] [--resizable-target-object-name <name>] [--resizable-target-unique-id <id>] [--add-line-feeds-target-object-name <name>] [--add-line-feeds-target-unique-id <id>] [--always-on-top-target-object-name <name>] [--always-on-top-target-unique-id <id>] [--always-on-bottom-target-object-name <name>] [--always-on-bottom-target-unique-id <id>] [--line <n>] [--column <n>] [--symbol <name>]");
    print_alternate_usage_line(catalog, "copperfin_studio_host --visual-object-list --path <asset> [--json]");
    print_alternate_usage_line(catalog, "copperfin_studio_host --visual-object-children --path <asset> [--record <n>] [--object-name <name>] [--unique-id <id>] [--json]");
    print_alternate_usage_line(catalog, "copperfin_studio_host --visual-object-descendants --path <asset> [--record <n>] [--object-name <name>] [--unique-id <id>] [--json]");
    print_alternate_usage_line(catalog, "copperfin_studio_host --visual-object-ancestors --path <asset> [--record <n>] [--object-name <name>] [--unique-id <id>] [--json]");
    print_alternate_usage_line(catalog, "copperfin_studio_host --visual-object-duplicate-batch --path <asset> (--selected-record <n>|--selected-object-name <name>|--selected-unique-id <id>) [--new-object-name <name>] [--new-name <name>] [--new-unique-id <id>] ... [--json]");
    print_alternate_usage_line(catalog, "copperfin_studio_host --visual-object-duplicate-subtree --path <asset> (--record <n>|--object-name <name>|--unique-id <id>) --replacement-source-unique-id <id> --new-object-name <name> --new-name <name> --new-unique-id <id> ... [--json]");
    print_alternate_usage_line(catalog, "copperfin_studio_host --visual-object-rename-batch --path <asset> (--selected-record <n>|--selected-object-name <name>|--selected-unique-id <id>) [--new-object-name <name>] [--new-name <name>] [--new-unique-id <id>] ... [--json]");
    print_alternate_usage_line(catalog, "copperfin_studio_host --visual-object-reorder-batch --path <asset> (--selected-record <n>|--selected-object-name <name>|--selected-unique-id <id>) --placement <front|back|before|after> [--target-object-name <name>] [--target-unique-id <id>] ... [--json]");
    print_alternate_usage_line(catalog, "copperfin_studio_host --visual-object-reparent-batch --path <asset> (--selected-record <n>|--selected-object-name <name>|--selected-unique-id <id>) [--parent-name <name>] [--parent-unique-id <id>] [--clear-parent] ... [--json]");
    print_alternate_usage_line(catalog, "copperfin_studio_host --visual-object-update-batch --path <asset> (--selected-record <n>|--selected-object-name <name>|--selected-unique-id <id>) --property-name <name> --property-value <value> ... [--json]");
    print_alternate_usage_line(catalog, "copperfin_studio_host --visual-method-list --path <asset> [--record <n>] [--object-name <name>] [--unique-id <id>] [--json]");
    print_alternate_usage_line(catalog, "copperfin_studio_host --visual-method-query --path <asset> [--record <n>] [--object-name <name>] [--unique-id <id>] --method-name <name> [--json]");
    print_alternate_usage_line(catalog, "copperfin_studio_host --visual-method-update --path <asset> [--record <n>] [--object-name <name>] [--unique-id <id>] --method-name <name> --method-kind <procedure|function> --method-source <text> [--json]");
    print_alternate_usage_line(catalog, "copperfin_studio_host --visual-method-delete --path <asset> [--record <n>] [--object-name <name>] [--unique-id <id>] --method-name <name> [--json]");
    print_alternate_usage_line(catalog, "copperfin_studio_host --visual-method-delete-batch --path <asset> --method-name <name> [--record <n>] [--object-name <name>] [--unique-id <id>] ... [--json]");
    print_alternate_usage_line(catalog, "copperfin_studio_host --visual-method-rename --path <asset> [--record <n>] [--object-name <name>] [--unique-id <id>] --method-name <name> --new-method-name <name> [--json]");
    print_alternate_usage_line(catalog, "copperfin_studio_host --visual-method-rename-batch --path <asset> --method-name <name> --new-method-name <name> [--record <n>] [--object-name <name>] [--unique-id <id>] ... [--json]");
    print_alternate_usage_line(catalog, "copperfin_studio_host --visual-method-copy --path <asset> [--source-record <n>] [--source-object-name <name>] [--source-unique-id <id>] --method-name <name> [--target-record <n>] [--target-object-name <name>] [--target-unique-id <id>] [--target-method-name <name>] [--replace-existing <true|false>] [--json]");
    print_alternate_usage_line(catalog, "copperfin_studio_host --visual-method-copy-batch --path <asset> --method-name <name> [--source-record <n>] [--source-object-name <name>] [--source-unique-id <id>] [--target-record <n>] [--target-object-name <name>] [--target-unique-id <id>] [--target-method-name <name>] [--replace-existing <true|false>] ... [--json]");
    print_alternate_usage_line(catalog, "copperfin_studio_host --visual-method-move --path <asset> [--source-record <n>] [--source-object-name <name>] [--source-unique-id <id>] --method-name <name> [--target-record <n>] [--target-object-name <name>] [--target-unique-id <id>] [--target-method-name <name>] [--replace-existing <true|false>] [--json]");
    print_alternate_usage_line(catalog, "copperfin_studio_host --visual-method-move-batch --path <asset> --method-name <name> [--source-record <n>] [--source-object-name <name>] [--source-unique-id <id>] [--target-record <n>] [--target-object-name <name>] [--target-unique-id <id>] [--target-method-name <name>] [--replace-existing <true|false>] ... [--json]");
    print_alternate_usage_line(catalog, "copperfin_studio_host --visual-method-reorder --path <asset> [--record <n>] [--object-name <name>] [--unique-id <id>] --method-name <name> --placement <first|last|before|after> [--relative-method-name <name>] [--json]");
    print_alternate_usage_line(catalog, "copperfin_studio_host --visual-method-reorder-batch --path <asset> --method-name <name> --placement <first|last|before|after> [--record <n>] [--object-name <name>] [--unique-id <id>] [--relative-method-name <name>] ... [--json]");
    print_alternate_usage_line(catalog, "copperfin_studio_host --visual-property-list --path <asset> [--record <n>] [--object-name <name>] [--unique-id <id>] [--json]");
    print_alternate_usage_line(catalog, "copperfin_studio_host --visual-property-query --path <asset> [--record <n>] [--object-name <name>] [--unique-id <id>] --property-name <name> [--json]");
    print_alternate_usage_line(catalog, "copperfin_studio_host --visual-property-update-batch --path <asset> [--record <n>] [--object-name <name>] [--unique-id <id>] --property-name <name> --property-value <value> ... [--json]");
    print_alternate_usage_line(catalog, "copperfin_studio_host --visual-property-filter --path <asset> [--record <n>] [--object-name <name>] [--unique-id <id>] [--property-filter-text <text>] [--json]");
    print_alternate_usage_line(catalog, "copperfin_studio_host --visual-property-clear --path <asset> [--record <n>] [--object-name <name>] [--unique-id <id>] --property-name <name> [--json]");
    print_alternate_usage_line(catalog, "copperfin_studio_host --visual-property-clear-batch --path <asset> --property-name <name> [--record <n>] [--object-name <name>] [--unique-id <id>] ... [--json]");
    print_alternate_usage_line(catalog, "copperfin_studio_host --visual-property-copy --path <asset> [--source-record <n>] [--source-object-name <name>] [--source-unique-id <id>] --property-name <name> [--target-record <n>] [--target-object-name <name>] [--target-unique-id <id>] [--target-property-name <name>] [--replace-existing <true|false>] [--json]");
    print_alternate_usage_line(catalog, "copperfin_studio_host --visual-property-copy-batch --path <asset> --property-name <name> [--source-record <n>] [--source-object-name <name>] [--source-unique-id <id>] [--target-record <n>] [--target-object-name <name>] [--target-unique-id <id>] [--target-property-name <name>] [--replace-existing <true|false>] ... [--json]");
    print_alternate_usage_line(catalog, "copperfin_studio_host --visual-property-move --path <asset> [--source-record <n>] [--source-object-name <name>] [--source-unique-id <id>] --property-name <name> [--target-record <n>] [--target-object-name <name>] [--target-unique-id <id>] [--target-property-name <name>] [--replace-existing <true|false>] [--json]");
    print_alternate_usage_line(catalog, "copperfin_studio_host --visual-property-move-batch --path <asset> --property-name <name> [--source-record <n>] [--source-object-name <name>] [--source-unique-id <id>] [--target-record <n>] [--target-object-name <name>] [--target-unique-id <id>] [--target-property-name <name>] [--replace-existing <true|false>] ... [--json]");
    print_alternate_usage_line(catalog, "copperfin_studio_host --visual-property-rename --path <asset> [--record <n>] [--object-name <name>] [--unique-id <id>] --property-name <name> --new-property-name <name> [--json]");
    print_alternate_usage_line(catalog, "copperfin_studio_host --visual-property-rename-batch --path <asset> --property-name <name> --new-property-name <name> [--record <n>] [--object-name <name>] [--unique-id <id>] ... [--json]");
    print_alternate_usage_line(catalog, "copperfin_studio_host --visual-property-reorder --path <asset> [--record <n>] [--object-name <name>] [--unique-id <id>] --property-name <name> --placement <first|last|before|after> [--relative-property-name <name>] [--json]");
    print_alternate_usage_line(catalog, "copperfin_studio_host --visual-property-reorder-batch --path <asset> --property-name <name> --placement <first|last|before|after> [--relative-property-name <name>] [--record <n>] [--object-name <name>] [--unique-id <id>] ... [--json]");
    print_alternate_usage_line(catalog, "copperfin_studio_host --builder-launch-plan <id> (--builder-context <token>|--selection-context <token>) [--path <asset>] [--record <n>] [--object-name <name>] [--unique-id <id>] [--json]");
    print_alternate_usage_line(catalog, "copperfin_studio_host --builder-launch-catalog --builder-context <token> [--path <asset>] [--record <n>] [--object-name <name>] [--unique-id <id>] [--json]");
    print_alternate_usage_line(catalog, "copperfin_studio_host --selection-builder-launch-catalog --selection-context <token> [--path <asset>] [--record <n>] [--object-name <name>] [--unique-id <id>] [--json]");
    print_alternate_usage_line(catalog, "copperfin_studio_host --builder-invocation-admission <id> (--builder-context <token>|--selection-context <token>) [--path <asset>] [--record <n>] [--object-name <name>] [--unique-id <id>] [--admit-ui-launch <true|false>] [--json]");
    print_alternate_usage_line(catalog, "copperfin_studio_host --builder-invocation-admission-catalog --builder-context <token> [--path <asset>] [--record <n>] [--object-name <name>] [--unique-id <id>] [--admit-ui-launch <true|false>] [--json]");
    print_alternate_usage_line(catalog, "copperfin_studio_host --selection-builder-invocation-admission-catalog --selection-context <token> [--path <asset>] [--record <n>] [--object-name <name>] [--unique-id <id>] [--admit-ui-launch <true|false>] [--json]");
    print_alternate_usage_line(catalog, "copperfin_studio_host --builder-dispatch <id> (--builder-context <token>|--selection-context <token>) [--path <asset>] [--record <n>] [--object-name <name>] [--unique-id <id>] [--admit-ui-launch <true|false>] [--json]");
    print_alternate_usage_line(catalog, "copperfin_studio_host --builder-execute <id> (--builder-context <token>|--selection-context <token>) --builder-launch-command <command> [--path <asset>] [--record <n>] [--object-name <name>] [--unique-id <id>] [--admit-ui-launch <true|false>] [--admit-builder-execution <true|false>] [--json]");
    print_alternate_usage_line(catalog, "copperfin_studio_host --builder-dispatch-catalog --builder-context <token> [--path <asset>] [--record <n>] [--object-name <name>] [--unique-id <id>] [--admit-ui-launch <true|false>] [--json]");
    print_alternate_usage_line(catalog, "copperfin_studio_host --builder-dispatch-execution-catalog --builder-context <token> [--path <asset>] [--record <n>] [--object-name <name>] [--unique-id <id>] [--admit-ui-launch <true|false>] [--admit-builder-execution <true|false>] [--json]");
    print_alternate_usage_line(catalog, "copperfin_studio_host --selection-builder-dispatch-catalog --selection-context <token> [--path <asset>] [--record <n>] [--object-name <name>] [--unique-id <id>] [--admit-ui-launch <true|false>] [--json]");
    print_alternate_usage_line(catalog, "copperfin_studio_host --selection-builder-dispatch-execution-catalog --selection-context <token> [--path <asset>] [--record <n>] [--object-name <name>] [--unique-id <id>] [--admit-ui-launch <true|false>] [--admit-builder-execution <true|false>] [--json]");
    print_alternate_usage_line(catalog, "copperfin_studio_host --editor-action-launch-plan <id> --selection-context <token> [--path <asset>] [--record <n>] [--object-name <name>] [--unique-id <id>] [--symbol <name>] [--line <n>] [--column <n>] [--json]");
    print_alternate_usage_line(catalog, "copperfin_studio_host --editor-action-launch-catalog --selection-context <token> [--path <asset>] [--record <n>] [--object-name <name>] [--unique-id <id>] [--symbol <name>] [--line <n>] [--column <n>] [--json]");
    print_alternate_usage_line(catalog, "copperfin_studio_host --editor-action-invocation-admission <id> --selection-context <token> [--path <asset>] [--record <n>] [--object-name <name>] [--unique-id <id>] [--symbol <name>] [--line <n>] [--column <n>] [--admit-editor-invocation <true|false>] [--json]");
    print_alternate_usage_line(catalog, "copperfin_studio_host --editor-action-invocation-admission-catalog --selection-context <token> [--path <asset>] [--record <n>] [--object-name <name>] [--unique-id <id>] [--symbol <name>] [--line <n>] [--column <n>] [--admit-editor-invocation <true|false>] [--json]");
    print_alternate_usage_line(catalog, "copperfin_studio_host --editor-action-dispatch <id> --selection-context <token> [--path <asset>] [--record <n>] [--object-name <name>] [--unique-id <id>] [--symbol <name>] [--line <n>] [--column <n>] [--admit-editor-invocation <true|false>] [--json]");
    print_alternate_usage_line(catalog, "copperfin_studio_host --editor-action-execute <id> --selection-context <token> --editor-action-launch-command <command> [--path <asset>] [--record <n>] [--object-name <name>] [--unique-id <id>] [--symbol <name>] [--line <n>] [--column <n>] [--admit-editor-invocation <true|false>] [--admit-editor-action-execution <true|false>] [--json]");
    print_alternate_usage_line(catalog, "copperfin_studio_host --editor-action-dispatch-catalog --selection-context <token> [--path <asset>] [--record <n>] [--object-name <name>] [--unique-id <id>] [--symbol <name>] [--line <n>] [--column <n>] [--admit-editor-invocation <true|false>] [--json]");
    print_alternate_usage_line(catalog, "copperfin_studio_host --editor-action-dispatch-execution-catalog --selection-context <token> [--path <asset>] [--record <n>] [--object-name <name>] [--unique-id <id>] [--symbol <name>] [--line <n>] [--column <n>] [--admit-editor-invocation <true|false>] [--admit-editor-action-execution <true|false>] [--json]");
    print_alternate_usage_line(catalog, "copperfin_studio_host --toolbox-palette-query --toolbox-context <token> [--toolbox-search <text>] [--toolbox-category <text>] [--json]");
    print_alternate_usage_line(catalog, "copperfin_studio_host --toolbox-palette-launch-plan --selection-context <token> [--path <asset>] [--record <n>] [--object-name <name>] [--unique-id <id>] [--json]");
    print_alternate_usage_line(catalog, "copperfin_studio_host --toolbox-palette-launch-catalog [--path <asset>] [--record <n>] [--object-name <name>] [--unique-id <id>] [--json]");
    print_alternate_usage_line(catalog, "copperfin_studio_host --toolbox-invocation-admission --selection-context <token> [--path <asset>] [--record <n>] [--object-name <name>] [--unique-id <id>] [--admit-palette-invocation <true|false>] [--json]");
    print_alternate_usage_line(catalog, "copperfin_studio_host --toolbox-invocation-admission-catalog --toolbox-context <token> [--path <asset>] [--record <n>] [--object-name <name>] [--unique-id <id>] [--admit-palette-invocation <true|false>] [--json]");
    print_alternate_usage_line(catalog, "copperfin_studio_host --selection-toolbox-invocation-admission-catalog --selection-context <token> [--path <asset>] [--record <n>] [--object-name <name>] [--unique-id <id>] [--admit-palette-invocation <true|false>] [--json]");
    print_alternate_usage_line(catalog, "copperfin_studio_host --toolbox-dispatch --selection-context <token> [--path <asset>] [--record <n>] [--object-name <name>] [--unique-id <id>] [--admit-palette-invocation <true|false>] [--json]");
    print_alternate_usage_line(catalog, "copperfin_studio_host --toolbox-execute --selection-context <token> --toolbox-launch-command <command> [--path <asset>] [--record <n>] [--object-name <name>] [--unique-id <id>] [--admit-palette-invocation <true|false>] [--admit-toolbox-execution <true|false>] [--json]");
    print_alternate_usage_line(catalog, "copperfin_studio_host --toolbox-dispatch-catalog --toolbox-context <token> [--path <asset>] [--record <n>] [--object-name <name>] [--unique-id <id>] [--admit-palette-invocation <true|false>] [--json]");
    print_alternate_usage_line(catalog, "copperfin_studio_host --toolbox-dispatch-execution-catalog --toolbox-context <token> [--path <asset>] [--record <n>] [--object-name <name>] [--unique-id <id>] [--admit-palette-invocation <true|false>] [--admit-toolbox-execution <true|false>] [--json]");
    print_alternate_usage_line(catalog, "copperfin_studio_host --selection-toolbox-dispatch-catalog --selection-context <token> [--path <asset>] [--record <n>] [--object-name <name>] [--unique-id <id>] [--admit-palette-invocation <true|false>] [--json]");
    print_alternate_usage_line(catalog, "copperfin_studio_host --selection-toolbox-dispatch-execution-catalog --selection-context <token> [--path <asset>] [--record <n>] [--object-name <name>] [--unique-id <id>] [--admit-palette-invocation <true|false>] [--admit-toolbox-execution <true|false>] [--json]");
    print_alternate_usage_line(catalog, "copperfin_studio_host --designer-launch-surfaces --selection-context <token> [--path <asset>] [--record <n>] [--object-name <name>] [--unique-id <id>] [--symbol <name>] [--line <n>] [--column <n>] [--json]");
    print_alternate_usage_line(catalog, "copperfin_studio_host --designer-invocation-admission --selection-context <token> [--path <asset>] [--record <n>] [--object-name <name>] [--unique-id <id>] [--symbol <name>] [--line <n>] [--column <n>] [--admit-editor-invocations <true|false>] [--admit-builder-invocations <true|false>] [--admit-toolbox-invocation <true|false>] [--json]");
    print_alternate_usage_line(catalog, "copperfin_studio_host --designer-dispatch --selection-context <token> [--path <asset>] [--record <n>] [--object-name <name>] [--unique-id <id>] [--symbol <name>] [--line <n>] [--column <n>] [--admit-editor-invocations <true|false>] [--admit-builder-invocations <true|false>] [--admit-toolbox-invocation <true|false>] [--json]");
    print_alternate_usage_line(catalog, "copperfin_studio_host --designer-execute --selection-context <token> [--path <asset>] [--record <n>] [--object-name <name>] [--unique-id <id>] [--symbol <name>] [--line <n>] [--column <n>] [--admit-editor-invocations <true|false>] [--admit-builder-invocations <true|false>] [--admit-toolbox-invocation <true|false>] [--admit-designer-execution <true|false>] [--editor-action-launch-command <command>] [--builder-launch-command <command>] [--toolbox-launch-command <command>] [--json]");
    print_alternate_usage_line(catalog, "copperfin_studio_host --designer-launch-surface-catalog [--path <asset>] [--record <n>] [--object-name <name>] [--unique-id <id>] [--symbol <name>] [--line <n>] [--column <n>] [--json]");
    print_alternate_usage_line(catalog, "copperfin_studio_host --designer-invocation-admission-catalog [--path <asset>] [--record <n>] [--object-name <name>] [--unique-id <id>] [--symbol <name>] [--line <n>] [--column <n>] [--admit-editor-invocations <true|false>] [--admit-builder-invocations <true|false>] [--admit-toolbox-invocation <true|false>] [--json]");
    print_alternate_usage_line(catalog, "copperfin_studio_host --designer-dispatch-catalog [--path <asset>] [--record <n>] [--object-name <name>] [--unique-id <id>] [--symbol <name>] [--line <n>] [--column <n>] [--admit-editor-invocations <true|false>] [--admit-builder-invocations <true|false>] [--admit-toolbox-invocation <true|false>] [--json]");
    print_alternate_usage_line(catalog, "copperfin_studio_host --designer-dispatch-execution-catalog [--path <asset>] [--record <n>] [--object-name <name>] [--unique-id <id>] [--symbol <name>] [--line <n>] [--column <n>] [--admit-editor-invocations <true|false>] [--admit-builder-invocations <true|false>] [--admit-toolbox-invocation <true|false>] [--admit-designer-execution <true|false>] [--json]");
    print_alternate_usage_line(catalog, "copperfin_studio_host --path <asset> --toolbox-create-plan <id> [--toolbox-context <token>] [--object-name <name>] [--unique-id <id>] [--parent-name <name>] [--field-value <name=value>] [--json]");
    print_alternate_usage_line(catalog, "copperfin_studio_host --path <asset> --selection-toolbox-create-plan <id> --selection-context <token> [--object-name <name>] [--unique-id <id>] [--parent-name <name>] [--field-value <name=value>] [--json]");
    print_alternate_usage_line(catalog, "copperfin_studio_host --path <asset> --toolbox-create-from-dispatch-plan <id> --selection-context <token> [--record <n>] [--object-name <selected>] [--unique-id <selected>] [--create-object-name <name>] [--create-unique-id <id>] [--create-parent-name <name>] [--field-value <name=value>] [--admit-palette-invocation <true|false>] [--json]");
    print_alternate_usage_line(catalog, "copperfin_studio_host --path <asset> --toolbox-create-from-dispatch <id> --selection-context <token> [--record <n>] [--object-name <selected>] [--unique-id <selected>] [--create-object-name <name>] [--create-unique-id <id>] [--create-parent-name <name>] [--field-value <name=value>] [--admit-palette-invocation <true|false>] [--json]");
    print_alternate_usage_line(catalog, "copperfin_studio_host --path <asset> --toolbox-create-dispatch-plan <id> [--toolbox-context <token>] [--object-name <name>] [--unique-id <id>] [--parent-name <name>] [--field-value <name=value>] [--admit-create-operation <true|false>] [--json]");
    print_alternate_usage_line(catalog, "copperfin_studio_host --path <asset> --selection-toolbox-create-dispatch-plan <id> --selection-context <token> [--object-name <name>] [--unique-id <id>] [--parent-name <name>] [--field-value <name=value>] [--admit-create-operation <true|false>] [--json]");
    print_alternate_usage_line(catalog, "copperfin_studio_host --path <asset> --toolbox-create-dispatch-from-dispatch-plan <id> --selection-context <token> [--record <n>] [--object-name <selected>] [--unique-id <selected>] [--create-object-name <name>] [--create-unique-id <id>] [--create-parent-name <name>] [--field-value <name=value>] [--admit-palette-invocation <true|false>] [--admit-create-operation <true|false>] [--json]");
    print_alternate_usage_line(catalog, "copperfin_studio_host --path <asset> --toolbox-create-batch-plan --toolbox-item <id> [--toolbox-context <token>] [--object-name <name>] [--unique-id <id>] [--parent-name <name>] [--field-value <name=value>] ... [--json]");
    print_alternate_usage_line(catalog, "copperfin_studio_host --path <asset> --selection-toolbox-create-batch-plan --selection-context <token> --toolbox-item <id> [--object-name <name>] [--unique-id <id>] [--parent-name <name>] [--field-value <name=value>] ... [--json]");
    print_alternate_usage_line(catalog, "copperfin_studio_host --path <asset> --selection-toolbox-create-batch --selection-context <token> --toolbox-item <id> [--object-name <name>] [--unique-id <id>] [--parent-name <name>] [--field-value <name=value>] ... [--json]");
    print_alternate_usage_line(catalog, "copperfin_studio_host --path <asset> --toolbox-create-batch-from-dispatch-plan --toolbox-item <id> --selection-context <token> [--record <n>] [--object-name <selected>] [--unique-id <selected>] [--create-object-name <name>] [--create-unique-id <id>] [--create-parent-name <name>] [--field-value <name=value>] ... [--admit-palette-invocation <true|false>] [--json]");
    print_alternate_usage_line(catalog, "copperfin_studio_host --path <asset> --toolbox-create-batch-from-dispatch --toolbox-item <id> --selection-context <token> [--record <n>] [--object-name <selected>] [--unique-id <selected>] [--create-object-name <name>] [--create-unique-id <id>] [--create-parent-name <name>] [--field-value <name=value>] ... [--admit-palette-invocation <true|false>] [--json]");
    print_alternate_usage_line(catalog, "copperfin_studio_host --path <asset> --toolbox-create-batch-dispatch-plan --toolbox-item <id> [--toolbox-context <token>] [--object-name <name>] [--unique-id <id>] [--parent-name <name>] [--field-value <name=value>] ... [--admit-create-operation <true|false>] [--json]");
    print_alternate_usage_line(catalog, "copperfin_studio_host --path <asset> --selection-toolbox-create-batch-dispatch-plan --selection-context <token> --toolbox-item <id> [--object-name <name>] [--unique-id <id>] [--parent-name <name>] [--field-value <name=value>] ... [--admit-create-operation <true|false>] [--json]");
    print_alternate_usage_line(catalog, "copperfin_studio_host --path <asset> --toolbox-create-batch-dispatch-from-dispatch-plan --toolbox-item <id> --selection-context <token> [--record <n>] [--object-name <selected>] [--unique-id <selected>] [--create-object-name <name>] [--create-unique-id <id>] [--create-parent-name <name>] [--field-value <name=value>] ... [--admit-palette-invocation <true|false>] [--admit-create-operation <true|false>] [--json]");
    print_alternate_usage_line(catalog, "copperfin_studio_host --path <asset> --toolbox-create-plan-catalog --toolbox-context <token> [--parent-name <name>] [--field-value <name=value>] [--json]");
    print_alternate_usage_line(catalog, "copperfin_studio_host --path <asset> --selection-toolbox-create-plan-catalog --selection-context <token> [--parent-name <name>] [--field-value <name=value>] [--json]");
    print_alternate_usage_line(catalog, "copperfin_studio_host --path <asset> --toolbox-create-batch-plan-catalog --toolbox-context <token> [--parent-name <name>] [--field-value <name=value>] [--json]");
    print_alternate_usage_line(catalog, "copperfin_studio_host --path <asset> --selection-toolbox-create-batch-plan-catalog --selection-context <token> [--parent-name <name>] [--field-value <name=value>] [--json]");
    print_alternate_usage_line(catalog, "copperfin_studio_host --path <asset> --toolbox-create-dispatch-catalog --toolbox-context <token> [--parent-name <name>] [--field-value <name=value>] [--admit-create-operation <true|false>] [--json]");
    print_alternate_usage_line(catalog, "copperfin_studio_host --path <asset> --selection-toolbox-create-dispatch-catalog --selection-context <token> [--parent-name <name>] [--field-value <name=value>] [--admit-create-operation <true|false>] [--json]");
    print_alternate_usage_line(catalog, "copperfin_studio_host --path <asset> --toolbox-create-batch-dispatch-catalog --toolbox-context <token> [--parent-name <name>] [--field-value <name=value>] [--admit-create-operation <true|false>] [--json]");
    print_alternate_usage_line(catalog, "copperfin_studio_host --path <asset> --selection-toolbox-create-batch-dispatch-catalog --selection-context <token> [--parent-name <name>] [--field-value <name=value>] [--admit-create-operation <true|false>] [--json]");
    print_alternate_usage_line(catalog, "copperfin_studio_host --path <asset> --toolbox-create-batch --toolbox-item <id> [--toolbox-context <token>] [--object-name <name>] [--unique-id <id>] [--parent-name <name>] [--field-value <name=value>] ... [--json]");
    print_alternate_usage_line(catalog, "copperfin_studio_host --path <asset> --toolbox-create <id> [--toolbox-context <token>] [--object-name <name>] [--unique-id <id>] [--parent-name <name>] [--field-value <name=value>] [--json]");
    print_alternate_usage_line(catalog, "copperfin_studio_host --path <asset> --selection-toolbox-create <id> --selection-context <token> [--object-name <name>] [--unique-id <id>] [--parent-name <name>] [--field-value <name=value>] [--json]");
    print_localized_object_usage_line(catalog, "StudioHost.LaunchParse.ObjectAssignment.DisplayValueTitle", "--display-value-object --display-value <value> [--display-value-target-object-name <name>] [--display-value-target-unique-id <id>]");
    print_localized_object_usage_line(catalog, "StudioHost.LaunchParse.ObjectAssignment.SelectedBackColorTitle", "--selected-back-color-object --selected-back-color <n> [--selected-back-color-target-object-name <name>] [--selected-back-color-target-unique-id <id>]");
    print_localized_object_usage_line(catalog, "StudioHost.LaunchParse.ObjectAssignment.SelectedForeColorTitle", "--selected-fore-color-object --selected-fore-color <n> [--selected-fore-color-target-object-name <name>] [--selected-fore-color-target-unique-id <id>]");
    print_localized_object_usage_line(catalog, "StudioHost.LaunchParse.ObjectAssignment.SelectedItemBackColorTitle", "--selected-item-back-color-object --selected-item-back-color <n> [--selected-item-back-color-target-object-name <name>] [--selected-item-back-color-target-unique-id <id>]");
    print_localized_object_usage_line(catalog, "StudioHost.LaunchParse.ObjectAssignment.SelectedItemForeColorTitle", "--selected-item-fore-color-object --selected-item-fore-color <n> [--selected-item-fore-color-target-object-name <name>] [--selected-item-fore-color-target-unique-id <id>]");
    print_localized_object_usage_line(catalog, "StudioHost.LaunchParse.ObjectAssignment.DisabledItemBackColorTitle", "--disabled-item-back-color-object --disabled-item-back-color <n> [--disabled-item-back-color-target-object-name <name>] [--disabled-item-back-color-target-unique-id <id>]");
    print_localized_object_usage_line(catalog, "StudioHost.LaunchParse.ObjectAssignment.DisabledItemForeColorTitle", "--disabled-item-fore-color-object --disabled-item-fore-color <n> [--disabled-item-fore-color-target-object-name <name>] [--disabled-item-fore-color-target-unique-id <id>]");
    print_localized_object_usage_line(catalog, "StudioHost.LaunchParse.ObjectAssignment.ItemBackColorTitle", "--item-back-color-object --item-back-color <n> [--item-back-color-target-object-name <name>] [--item-back-color-target-unique-id <id>]");
    print_localized_object_usage_line(catalog, "StudioHost.LaunchParse.ObjectAssignment.ItemForeColorTitle", "--item-fore-color-object --item-fore-color <n> [--item-fore-color-target-object-name <name>] [--item-fore-color-target-unique-id <id>]");
    print_localized_object_usage_line(catalog, "StudioHost.LaunchParse.ObjectAssignment.HighlightBackColorTitle", "--highlight-back-color-object --highlight-back-color <n> [--highlight-back-color-target-object-name <name>] [--highlight-back-color-target-unique-id <id>]");
    print_localized_object_usage_line(catalog, "StudioHost.LaunchParse.ObjectAssignment.HighlightForeColorTitle", "--highlight-fore-color-object --highlight-fore-color <n> [--highlight-fore-color-target-object-name <name>] [--highlight-fore-color-target-unique-id <id>]");
    print_localized_object_usage_line(catalog, "StudioHost.LaunchParse.ObjectAssignment.BackColorTitle", "--back-color-object --back-color <n> [--back-color-target-object-name <name>] [--back-color-target-unique-id <id>]");
    print_localized_object_usage_line(catalog, "StudioHost.LaunchParse.ObjectAssignment.ForeColorTitle", "--fore-color-object --fore-color <n> [--fore-color-target-object-name <name>] [--fore-color-target-unique-id <id>]");
    print_localized_object_usage_line(catalog, "StudioHost.LaunchParse.ObjectAssignment.DisabledBackColorTitle", "--disabled-back-color-object --disabled-back-color <n> [--disabled-back-color-target-object-name <name>] [--disabled-back-color-target-unique-id <id>]");
    print_localized_object_usage_line(catalog, "StudioHost.LaunchParse.ObjectAssignment.DisabledForeColorTitle", "--disabled-fore-color-object --disabled-fore-color <n> [--disabled-fore-color-target-object-name <name>] [--disabled-fore-color-target-unique-id <id>]");
    print_localized_object_usage_line(catalog, "StudioHost.LaunchParse.ObjectAssignment.DynamicBackColorTitle", "--dynamic-back-color-object --dynamic-back-color <expr> [--dynamic-back-color-target-object-name <name>] [--dynamic-back-color-target-unique-id <id>]");
    print_localized_object_usage_line(catalog, "StudioHost.LaunchParse.ObjectAssignment.DynamicForeColorTitle", "--dynamic-fore-color-object --dynamic-fore-color <expr> [--dynamic-fore-color-target-object-name <name>] [--dynamic-fore-color-target-unique-id <id>]");
    print_localized_object_usage_line(catalog, "StudioHost.LaunchParse.ObjectAssignment.OleDropMode", "--ole-drop-mode-object --ole-drop-mode <n> [--ole-drop-mode-target-object-name <name>] [--ole-drop-mode-target-unique-id <id>]");
    print_localized_object_usage_line(catalog, "StudioHost.LaunchParse.ObjectAssignment.OleDropEffects", "--ole-drop-effects-object --ole-drop-effects <n> [--ole-drop-effects-target-object-name <name>] [--ole-drop-effects-target-unique-id <id>]");
    print_localized_object_usage_line(catalog, "StudioHost.LaunchParse.ObjectAssignment.OleDropTextInsertion", "--ole-drop-text-insertion-object --ole-drop-text-insertion <n> [--ole-drop-text-insertion-target-object-name <name>] [--ole-drop-text-insertion-target-unique-id <id>]");
    print_localized_object_usage_line(catalog, "StudioHost.LaunchParse.ObjectAssignment.HideSelectionTitle", "--hide-selection-object --hide-selection <true|false> [--hide-selection-target-object-name <name>] [--hide-selection-target-unique-id <id>]");
    print_localized_object_usage_line(catalog, "StudioHost.LaunchParse.ObjectAssignment.BindControlsTitle", "--bind-controls-object --bind-controls <true|false> [--bind-controls-target-object-name <name>] [--bind-controls-target-unique-id <id>]");
    print_localized_object_usage_line(catalog, "StudioHost.LaunchParse.ObjectAssignment.AutoVerbMenuTitle", "--auto-verb-menu-object --auto-verb-menu <true|false> [--auto-verb-menu-target-object-name <name>] [--auto-verb-menu-target-unique-id <id>]");
    print_localized_object_usage_line(catalog, "StudioHost.LaunchParse.ObjectAssignment.DesktopTitle", "--desktop-object --desktop <true|false> [--desktop-target-object-name <name>] [--desktop-target-unique-id <id>]");
    print_localized_object_usage_line(catalog, "StudioHost.LaunchParse.ObjectAssignment.KeyPreviewTitle", "--key-preview-object --key-preview <true|false> [--key-preview-target-object-name <name>] [--key-preview-target-unique-id <id>]");
    print_localized_object_usage_line(catalog, "StudioHost.LaunchParse.ObjectAssignment.MacDesktopTitle", "--mac-desktop-object --mac-desktop <true|false> [--mac-desktop-target-object-name <name>] [--mac-desktop-target-unique-id <id>]");
    print_localized_object_usage_line(catalog, "StudioHost.LaunchParse.ObjectAssignment.MaxButtonTitle", "--max-button-object --max-button <true|false> [--max-button-target-object-name <name>] [--max-button-target-unique-id <id>]");
    print_localized_object_usage_line(catalog, "StudioHost.LaunchParse.ObjectAssignment.MinButtonTitle", "--min-button-object --min-button <true|false> [--min-button-target-object-name <name>] [--min-button-target-unique-id <id>]");
    print_localized_object_usage_line(catalog, "StudioHost.LaunchParse.ObjectAssignment.MinHeightTitle", "--min-height-object --min-height <n> [--min-height-target-object-name <name>] [--min-height-target-unique-id <id>]");
    print_localized_object_usage_line(catalog, "StudioHost.LaunchParse.ObjectAssignment.MinWidthTitle", "--min-width-object --min-width <n> [--min-width-target-object-name <name>] [--min-width-target-unique-id <id>]");
    print_localized_object_usage_line(catalog, "StudioHost.LaunchParse.ObjectAssignment.MaxHeightTitle", "--max-height-object --max-height <n> [--max-height-target-object-name <name>] [--max-height-target-unique-id <id>]");
    print_localized_object_usage_line(catalog, "StudioHost.LaunchParse.ObjectAssignment.MovableTitle", "--movable-object --movable <true|false> [--movable-target-object-name <name>] [--movable-target-unique-id <id>]");
    print_localized_object_usage_line(catalog, "StudioHost.LaunchParse.ObjectAssignment.HalfHeightCaptionTitle", "--half-height-caption-object --half-height-caption <true|false> [--half-height-caption-target-object-name <name>] [--half-height-caption-target-unique-id <id>]");
    print_localized_object_usage_line(catalog, "StudioHost.LaunchParse.ObjectAssignment.LinkMasterTitle", "--link-master-object --link-master <value> [--link-master-target-object-name <name>] [--link-master-target-unique-id <id>]");
    print_localized_object_usage_line(catalog, "StudioHost.LaunchParse.ObjectAssignment.MdiFormTitle", "--mdi-form-object --mdi-form <true|false> [--mdi-form-target-object-name <name>] [--mdi-form-target-unique-id <id>]");
    print_localized_object_usage_line(catalog, "StudioHost.LaunchParse.ObjectAssignment.BackStyleTitle", "--back-style-object --back-style <n> [--back-style-target-object-name <name>] [--back-style-target-unique-id <id>]");
    print_localized_object_usage_line(catalog, "StudioHost.LaunchParse.ObjectAssignment.BorderStyleTitle", "--border-style-object --border-style <n> [--border-style-target-object-name <name>] [--border-style-target-unique-id <id>]");
    print_localized_object_usage_line(catalog, "StudioHost.LaunchParse.ObjectAssignment.BorderWidthTitle", "--border-width-object --border-width <n> [--border-width-target-object-name <name>] [--border-width-target-unique-id <id>]");
    print_localized_object_usage_line(catalog, "StudioHost.LaunchParse.ObjectAssignment.BorderColorTitle", "--border-color-object --border-color <n> [--border-color-target-object-name <name>] [--border-color-target-unique-id <id>]");
    print_localized_object_usage_line(catalog, "StudioHost.LaunchParse.ObjectAssignment.SpecialEffectTitle", "--special-effect-object --special-effect <n> [--special-effect-target-object-name <name>] [--special-effect-target-unique-id <id>]");
    print_localized_object_usage_line(catalog, "StudioHost.LaunchParse.ObjectAssignment.ScrollBarsTitle", "--scroll-bars-object --scroll-bars <n> [--scroll-bars-target-object-name <name>] [--scroll-bars-target-unique-id <id>]");
    print_localized_object_usage_line(catalog, "StudioHost.LaunchParse.ObjectAssignment.WindowStateTitle", "--window-state-object --window-state <n> [--window-state-target-object-name <name>] [--window-state-target-unique-id <id>]");
    print_localized_object_usage_line(catalog, "StudioHost.LaunchParse.ObjectAssignment.ShowWindowTitle", "--show-window-object --show-window <n> [--show-window-target-object-name <name>] [--show-window-target-unique-id <id>]");
    print_localized_object_usage_line(catalog, "StudioHost.LaunchParse.ObjectAssignment.TitleBarTitle", "--title-bar-object --title-bar <n> [--title-bar-target-object-name <name>] [--title-bar-target-unique-id <id>]");
    print_localized_object_usage_line(catalog, "StudioHost.LaunchParse.ObjectAssignment.MousePointerTitle", "--mouse-pointer-object --mouse-pointer <n> [--mouse-pointer-target-object-name <name>] [--mouse-pointer-target-unique-id <id>]");
    print_localized_object_usage_line(catalog, "StudioHost.LaunchParse.ObjectAssignment.PictureMarginTitle", "--picture-margin-object --picture-margin <n> [--picture-margin-target-object-name <name>] [--picture-margin-target-unique-id <id>]");
    print_localized_object_usage_line(catalog, "StudioHost.LaunchParse.ObjectAssignment.PicturePositionTitle", "--picture-position-object --picture-position <n> [--picture-position-target-object-name <name>] [--picture-position-target-unique-id <id>]");
    print_localized_object_usage_line(catalog, "StudioHost.LaunchParse.ObjectAssignment.PictureSpacingTitle", "--picture-spacing-object --picture-spacing <n> [--picture-spacing-target-object-name <name>] [--picture-spacing-target-unique-id <id>]");
    print_localized_object_usage_line(catalog, "StudioHost.LaunchParse.ObjectAssignment.PictureSelectionDisplayTitle", "--picture-selection-display-object --picture-selection-display <n> [--picture-selection-display-target-object-name <name>] [--picture-selection-display-target-unique-id <id>]");
    print_localized_object_usage_line(catalog, "StudioHost.LaunchParse.ObjectAssignment.DynamicInputMaskTitle", "--dynamic-input-mask-object --dynamic-input-mask <expr> [--dynamic-input-mask-target-object-name <name>] [--dynamic-input-mask-target-unique-id <id>]");
    print_localized_object_usage_line(catalog, "StudioHost.LaunchParse.ObjectAssignment.DynamicLineHeightTitle", "--dynamic-line-height-object --dynamic-line-height <expr> [--dynamic-line-height-target-object-name <name>] [--dynamic-line-height-target-unique-id <id>]");
    print_localized_object_usage_line(catalog, "StudioHost.LaunchParse.ObjectAssignment.DynamicAlignmentTitle", "--dynamic-alignment-object --dynamic-alignment <expr> [--dynamic-alignment-target-object-name <name>] [--dynamic-alignment-target-unique-id <id>]");
    print_localized_object_usage_line(catalog, "StudioHost.LaunchParse.ObjectAssignment.FontNameTitle", "--font-name-object --font-name <value> [--font-name-target-object-name <name>] [--font-name-target-unique-id <id>]");
    print_localized_object_usage_line(catalog, "StudioHost.LaunchParse.ObjectAssignment.FontSizeTitle", "--font-size-object --font-size <n> [--font-size-target-object-name <name>] [--font-size-target-unique-id <id>]");
    print_localized_object_usage_line(catalog, "StudioHost.LaunchParse.ObjectAssignment.FontBoldTitle", "--font-bold-object --font-bold <true|false> [--font-bold-target-object-name <name>] [--font-bold-target-unique-id <id>]");
    print_localized_object_usage_line(catalog, "StudioHost.LaunchParse.ObjectAssignment.FontItalicTitle", "--font-italic-object --font-italic <true|false> [--font-italic-target-object-name <name>] [--font-italic-target-unique-id <id>]");
    print_localized_object_usage_line(catalog, "StudioHost.LaunchParse.ObjectAssignment.FontUnderlineTitle", "--font-underline-object --font-underline <true|false> [--font-underline-target-object-name <name>] [--font-underline-target-unique-id <id>]");
    print_localized_object_usage_line(catalog, "StudioHost.LaunchParse.ObjectAssignment.FontStrikethruTitle", "--font-strikethru-object --font-strikethru <true|false> [--font-strikethru-target-object-name <name>] [--font-strikethru-target-unique-id <id>]");
    print_localized_object_usage_line(catalog, "StudioHost.LaunchParse.ObjectAssignment.FontOutlineTitle", "--font-outline-object --font-outline <true|false> [--font-outline-target-object-name <name>] [--font-outline-target-unique-id <id>]");
    print_localized_object_usage_line(catalog, "StudioHost.LaunchParse.ObjectAssignment.FontShadowTitle", "--font-shadow-object --font-shadow <true|false> [--font-shadow-target-object-name <name>] [--font-shadow-target-unique-id <id>]");
    print_localized_object_usage_line(catalog, "StudioHost.LaunchParse.ObjectAssignment.MaxWidthTitle", "--max-width-object --max-width <n> [--max-width-target-object-name <name>] [--max-width-target-unique-id <id>]");
    print_localized_object_usage_line(catalog, "StudioHost.LaunchParse.ObjectAssignment.MaxLeftTitle", "--max-left-object --max-left <n> [--max-left-target-object-name <name>] [--max-left-target-unique-id <id>]");
    print_localized_object_usage_line(catalog, "StudioHost.LaunchParse.ObjectAssignment.MaxTopTitle", "--max-top-object --max-top <n> [--max-top-target-object-name <name>] [--max-top-target-unique-id <id>]");
    print_localized_object_usage_line(catalog, "StudioHost.LaunchParse.ObjectAssignment.ButtonCountTitle", "--button-count-object --button-count <n> [--button-count-target-object-name <name>] [--button-count-target-unique-id <id>]");
    print_localized_object_usage_line(catalog, "StudioHost.LaunchParse.ObjectAssignment.CurvatureTitle", "--curvature-object --curvature <n> [--curvature-target-object-name <name>] [--curvature-target-unique-id <id>]");
    print_localized_object_usage_line(catalog, "StudioHost.LaunchParse.ObjectAssignment.DrawModeTitle", "--draw-mode-object --draw-mode <n> [--draw-mode-target-object-name <name>] [--draw-mode-target-unique-id <id>]");
    print_localized_object_usage_line(catalog, "StudioHost.LaunchParse.ObjectAssignment.DrawStyleTitle", "--draw-style-object --draw-style <n> [--draw-style-target-object-name <name>] [--draw-style-target-unique-id <id>]");
    print_localized_object_usage_line(catalog, "StudioHost.LaunchParse.ObjectAssignment.DrawWidthTitle", "--draw-width-object --draw-width <n> [--draw-width-target-object-name <name>] [--draw-width-target-unique-id <id>]");
    print_localized_object_usage_line(catalog, "StudioHost.LaunchParse.ObjectAssignment.FillStyleTitle", "--fill-style-object --fill-style <n> [--fill-style-target-object-name <name>] [--fill-style-target-unique-id <id>]");
    print_localized_object_usage_line(catalog, "StudioHost.LaunchParse.ObjectAssignment.ScaleModeTitle", "--scale-mode-object --scale-mode <n> [--scale-mode-target-object-name <name>] [--scale-mode-target-unique-id <id>]");
    print_localized_object_usage_line(catalog, "StudioHost.LaunchParse.ObjectAssignment.BufferModeTitle", "--buffer-mode-object --buffer-mode <n> [--buffer-mode-target-object-name <name>] [--buffer-mode-target-unique-id <id>]");
    print_localized_object_usage_line(catalog, "StudioHost.LaunchParse.ObjectAssignment.BufferModeOverrideTitle", "--buffer-mode-override-object --buffer-mode-override <n> [--buffer-mode-override-target-object-name <name>] [--buffer-mode-override-target-unique-id <id>]");
    print_localized_object_usage_line(catalog, "StudioHost.LaunchParse.ObjectAssignment.DataSessionTitle", "--data-session-object --data-session <n> [--data-session-target-object-name <name>] [--data-session-target-unique-id <id>]");
    print_localized_object_usage_line(catalog, "StudioHost.LaunchParse.ObjectAssignment.GridLineColorTitle", "--grid-line-color-object --grid-line-color <n> [--grid-line-color-target-object-name <name>] [--grid-line-color-target-unique-id <id>]");
    print_localized_object_usage_line(catalog, "StudioHost.LaunchParse.ObjectAssignment.HeaderHeightTitle", "--header-height-object --header-height <n> [--header-height-target-object-name <name>] [--header-height-target-unique-id <id>]");
    print_localized_object_usage_line(catalog, "StudioHost.LaunchParse.ObjectAssignment.RowHeightTitle", "--row-height-object --row-height <n> [--row-height-target-object-name <name>] [--row-height-target-unique-id <id>]");
    print_localized_object_usage_line(catalog, "StudioHost.LaunchParse.ObjectAssignment.LockColumnsTitle", "--lock-columns-object --lock-columns <n> [--lock-columns-target-object-name <name>] [--lock-columns-target-unique-id <id>]");
    print_localized_object_usage_line(catalog, "StudioHost.LaunchParse.ObjectAssignment.LockColumnsLeftTitle", "--lock-columns-left-object --lock-columns-left <n> [--lock-columns-left-target-object-name <name>] [--lock-columns-left-target-unique-id <id>]");
    print_localized_object_usage_line(catalog, "StudioHost.LaunchParse.ObjectAssignment.GridLineWidthTitle", "--grid-line-width-object --grid-line-width <n> [--grid-line-width-target-object-name <name>] [--grid-line-width-target-unique-id <id>]");
    print_localized_object_usage_line(catalog, "StudioHost.LaunchParse.ObjectAssignment.GridLinesTitle", "--grid-lines-object --grid-lines <n> [--grid-lines-target-object-name <name>] [--grid-lines-target-unique-id <id>]");
    print_localized_object_usage_line(catalog, "StudioHost.LaunchParse.ObjectAssignment.HighlightRowLineWidthTitle", "--highlight-row-line-width-object --highlight-row-line-width <n> [--highlight-row-line-width-target-object-name <name>] [--highlight-row-line-width-target-unique-id <id>]");
    print_localized_object_usage_line(catalog, "StudioHost.LaunchParse.ObjectAssignment.PartitionTitle", "--partition-object --partition <n> [--partition-target-object-name <name>] [--partition-target-unique-id <id>]");
    print_localized_object_usage_line(catalog, "StudioHost.LaunchParse.ObjectAssignment.RecordSourceTypeTitle", "--record-source-type-object --record-source-type <n> [--record-source-type-target-object-name <name>] [--record-source-type-target-unique-id <id>]");
    print_localized_object_usage_line(catalog, "StudioHost.LaunchParse.ObjectAssignment.ColumnOrderTitle", "--column-order-object --column-order <n> [--column-order-target-object-name <name>] [--column-order-target-unique-id <id>]");
    print_localized_object_usage_line(catalog, "StudioHost.LaunchParse.ObjectAssignment.HighlightStyleTitle", "--highlight-style-object --highlight-style <n> [--highlight-style-target-object-name <name>] [--highlight-style-target-unique-id <id>]");
    print_localized_object_usage_line(catalog, "StudioHost.LaunchParse.ObjectAssignment.ChildOrderTitle", "--child-order-object --child-order <n> [--child-order-target-object-name <name>] [--child-order-target-unique-id <id>]");
    print_localized_object_usage_line(catalog, "StudioHost.LaunchParse.ObjectAssignment.FillColorTitle", "--fill-color-object --fill-color <n> [--fill-color-target-object-name <name>] [--fill-color-target-unique-id <id>]");
    print_localized_object_usage_line(catalog, "StudioHost.LaunchParse.ObjectAssignment.ListItemIdTitle", "--list-item-id-object --list-item-id <n> [--list-item-id-target-object-name <name>] [--list-item-id-target-unique-id <id>]");
    print_localized_object_usage_line(catalog, "StudioHost.LaunchParse.ObjectAssignment.TabOrientationTitle", "--tab-orientation-object --tab-orientation <n> [--tab-orientation-target-object-name <name>] [--tab-orientation-target-unique-id <id>]");
    print_localized_object_usage_line(catalog, "StudioHost.LaunchParse.ObjectAssignment.DisplayOrientationTitle", "--display-orientation-object --display-orientation <n> [--display-orientation-target-object-name <name>] [--display-orientation-target-unique-id <id>]");
    print_localized_object_usage_line(catalog, "StudioHost.LaunchParse.ObjectAssignment.HelpContextIdTitle", "--help-context-id-object --help-context-id <n> [--help-context-id-target-object-name <name>] [--help-context-id-target-unique-id <id>]");
    print_localized_object_usage_line(catalog, "StudioHost.LaunchParse.ObjectAssignment.WhatsThisHelpId", "--whats-this-help-id-object --whats-this-help-id <n> [--whats-this-help-id-target-object-name <name>] [--whats-this-help-id-target-unique-id <id>]");
    print_localized_object_usage_line(catalog, "StudioHost.LaunchParse.ObjectAssignment.WhatsThisHelp", "--whats-this-help-object --whats-this-help <true|false> [--whats-this-help-target-object-name <name>] [--whats-this-help-target-unique-id <id>]");
    print_localized_object_usage_line(catalog, "StudioHost.LaunchParse.ObjectAssignment.WhatsThisButton", "--whats-this-button-object --whats-this-button <true|false> [--whats-this-button-target-object-name <name>] [--whats-this-button-target-unique-id <id>]");
    print_localized_object_usage_line(catalog, "StudioHost.LaunchParse.ObjectAssignment.RecordSourceTitle", "--record-source-object --record-source <value> [--record-source-target-object-name <name>] [--record-source-target-unique-id <id>]");
    print_localized_object_usage_line(catalog, "StudioHost.LaunchParse.ObjectAssignment.FormSetClassTitle", "--form-set-class-object --form-set-class <value> [--form-set-class-target-object-name <name>] [--form-set-class-target-unique-id <id>]");
    print_localized_object_usage_line(catalog, "StudioHost.LaunchParse.ObjectAssignment.DefaultFilePathTitle", "--default-file-path-object --default-file-path <value> [--default-file-path-target-object-name <name>] [--default-file-path-target-unique-id <id>]");
    print_localized_object_usage_line(catalog, "StudioHost.LaunchParse.ObjectAssignment.InitialSelectedAliasTitle", "--initial-selected-alias-object --initial-selected-alias <value> [--initial-selected-alias-target-object-name <name>] [--initial-selected-alias-target-unique-id <id>]");
    print_alternate_usage_line(catalog, "copperfin_studio_host --list-subsystems [--json]");
    print_alternate_usage_line(catalog, "copperfin_studio_host <asset>");
    print_selection_context_tokens_line(catalog, "visual_object, visual_method, container_object, class_designer, report_expression, label_expression, menu_item, project_item, data_environment");
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

void print_json_string_array(const std::vector<std::string>& values) {
    std::cout << "[";
    for (std::size_t index = 0U; index < values.size(); ++index) {
        if (index != 0U) {
            std::cout << ", ";
        }
        print_json_string(values[index]);
    }
    std::cout << "]";
}

void print_json_int_array(const std::vector<int>& values) {
    std::cout << "[";
    for (std::size_t index = 0U; index < values.size(); ++index) {
        if (index != 0U) {
            std::cout << ", ";
        }
        std::cout << values[index];
    }
    std::cout << "]";
}

bool parse_size_t_token(const std::string& token, std::size_t& value) {
    if (token.empty() || token.front() == '-' || token.front() == '+') {
        return false;
    }
    try {
        std::size_t consumed = 0U;
        const auto parsed = std::stoull(token, &consumed, 10);
        if (consumed != token.size()) {
            return false;
        }
        value = static_cast<std::size_t>(parsed);
        return true;
    } catch (...) {
        return false;
    }
}

std::string lowercase_copy(std::string text) {
    std::transform(text.begin(), text.end(), text.begin(), [](unsigned char ch) {
        return static_cast<char>(std::tolower(ch));
    });
    return text;
}

bool parse_bool_token(std::string token, bool& value) {
    token = lowercase_copy(std::move(token));
    if (token == "true" || token == ".t." || token == "t" || token == "1" || token == "yes" || token == "on") {
        value = true;
        return true;
    }
    if (token == "false" || token == ".f." || token == "f" || token == "0" || token == "no" || token == "off") {
        value = false;
        return true;
    }
    return false;
}

bool parse_builder_context_token(
    const std::string& token,
    copperfin::studio::StudioBuilderContext& context) {
    for (const auto candidate : {
             copperfin::studio::StudioBuilderContext::form,
             copperfin::studio::StudioBuilderContext::class_designer,
             copperfin::studio::StudioBuilderContext::control,
             copperfin::studio::StudioBuilderContext::report,
             copperfin::studio::StudioBuilderContext::label,
             copperfin::studio::StudioBuilderContext::menu,
             copperfin::studio::StudioBuilderContext::project,
             copperfin::studio::StudioBuilderContext::data_environment
         }) {
        if (token == copperfin::studio::studio_builder_context_name(candidate)) {
            context = candidate;
            return true;
        }
    }
    return false;
}

bool parse_editor_selection_context_token(
    const std::string& token,
    copperfin::studio::StudioEditorSelectionContext& context) {
    for (const auto candidate : {
             copperfin::studio::StudioEditorSelectionContext::visual_object,
             copperfin::studio::StudioEditorSelectionContext::visual_method,
             copperfin::studio::StudioEditorSelectionContext::container_object,
             copperfin::studio::StudioEditorSelectionContext::class_designer,
             copperfin::studio::StudioEditorSelectionContext::report_expression,
             copperfin::studio::StudioEditorSelectionContext::label_expression,
             copperfin::studio::StudioEditorSelectionContext::menu_item,
             copperfin::studio::StudioEditorSelectionContext::project_item,
             copperfin::studio::StudioEditorSelectionContext::data_environment
         }) {
        if (token == copperfin::studio::studio_editor_selection_context_name(candidate)) {
            context = candidate;
            return true;
        }
    }
    return false;
}

BuilderLaunchPlanParseResult parse_builder_launch_plan_arguments(
    const copperfin::localization::LocalizedCatalog& catalog,
    const std::vector<std::string>& args) {
    BuilderLaunchPlanParseResult result{};
    result.output_json = std::find(args.begin(), args.end(), "--json") != args.end();
    result.requested = std::find(args.begin(), args.end(), "--builder-launch-plan") != args.end();
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
                fail(builder_parse_missing_value(catalog, option));
                return {};
            }
            ++index;
            return args[index];
        };

        if (argument == "--json") {
            continue;
        }
        if (argument == "--builder-launch-plan") {
            result.request.builder_id = require_value(argument);
        } else if (argument == "--builder-context") {
            const std::string token = require_value(argument);
            copperfin::studio::StudioBuilderContext parsed_context{};
            if (!parse_builder_context_token(token, parsed_context)) {
                fail(builder_parse_unknown_builder_context_token(catalog, token));
                continue;
            }
            result.context_provided = true;
            result.request.context = parsed_context;
        } else if (argument == "--selection-context") {
            const std::string token = require_value(argument);
            copperfin::studio::StudioEditorSelectionContext parsed_context{};
            if (!parse_editor_selection_context_token(token, parsed_context)) {
                fail(builder_parse_unknown_selection_context_token(catalog, token));
                continue;
            }
            result.selection_context_provided = true;
            result.selection_context = parsed_context;
        } else if (argument == "--path") {
            result.request.asset_path = require_value(argument);
        } else if (argument == "--record") {
            const std::string token = require_value(argument);
            std::size_t record_index = 0U;
            if (!parse_size_t_token(token, record_index)) {
                fail(builder_parse_record_non_negative_integer(catalog));
                continue;
            }
            result.request.record_index = record_index;
        } else if (argument == "--object-name") {
            result.request.object_name = require_value(argument);
        } else if (argument == "--unique-id") {
            result.request.unique_id = require_value(argument);
        } else {
            fail(builder_parse_unknown_option(catalog, "builder-launch-plan", argument));
        }
    }

    if (result.ok && result.request.builder_id.empty()) {
        fail(builder_parse_message(catalog, "StudioHost.BuilderParse.Error.NoBuilderId"));
    }
    if (result.ok && result.context_provided && result.selection_context_provided) {
        fail(builder_parse_context_conflict(catalog, "StudioHost.BuilderParse.RequestName.LaunchPlan"));
    }
    if (result.ok && !result.context_provided && !result.selection_context_provided) {
        fail(builder_parse_message(catalog, "StudioHost.BuilderParse.Error.NoBuilderOrSelectionContext"));
    }
    return result;
}

BuilderLaunchCatalogParseResult parse_builder_launch_catalog_arguments(
    const copperfin::localization::LocalizedCatalog& catalog,
    const std::vector<std::string>& args) {
    BuilderLaunchCatalogParseResult result{};
    result.output_json = std::find(args.begin(), args.end(), "--json") != args.end();
    result.requested = std::find(args.begin(), args.end(), "--builder-launch-catalog") != args.end();
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
                fail(builder_parse_missing_value(catalog, option));
                return {};
            }
            ++index;
            return args[index];
        };

        if (argument == "--json" || argument == "--builder-launch-catalog") {
            continue;
        }
        if (argument == "--builder-context") {
            const std::string token = require_value(argument);
            copperfin::studio::StudioBuilderContext parsed_context{};
            if (!parse_builder_context_token(token, parsed_context)) {
                fail(builder_parse_unknown_builder_context_token(catalog, token));
                continue;
            }
            result.context_provided = true;
            result.request.context = parsed_context;
        } else if (argument == "--path") {
            result.request.asset_path = require_value(argument);
        } else if (argument == "--record") {
            const std::string token = require_value(argument);
            std::size_t record_index = 0U;
            if (!parse_size_t_token(token, record_index)) {
                fail(builder_parse_record_non_negative_integer(catalog));
                continue;
            }
            result.request.record_index = record_index;
        } else if (argument == "--object-name") {
            result.request.object_name = require_value(argument);
        } else if (argument == "--unique-id") {
            result.request.unique_id = require_value(argument);
        } else {
            fail(builder_parse_unknown_option(catalog, "builder-launch-catalog", argument));
        }
    }

    if (result.ok && !result.context_provided) {
        fail(builder_parse_message(catalog, "StudioHost.BuilderParse.Error.NoBuilderContext"));
    }
    return result;
}

BuilderInvocationAdmissionParseResult parse_builder_invocation_admission_arguments(
    const copperfin::localization::LocalizedCatalog& catalog,
    const std::vector<std::string>& args) {
    BuilderInvocationAdmissionParseResult result{};
    result.output_json = std::find(args.begin(), args.end(), "--json") != args.end();
    result.requested = std::find(args.begin(), args.end(), "--builder-invocation-admission") != args.end();
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
                fail(builder_parse_missing_value(catalog, option));
                return {};
            }
            ++index;
            return args[index];
        };

        if (argument == "--json") {
            continue;
        }
        if (argument == "--builder-invocation-admission") {
            result.request.builder_id = require_value(argument);
        } else if (argument == "--builder-context") {
            const std::string token = require_value(argument);
            copperfin::studio::StudioBuilderContext parsed_context{};
            if (!parse_builder_context_token(token, parsed_context)) {
                fail(builder_parse_unknown_builder_context_token(catalog, token));
                continue;
            }
            result.context_provided = true;
            result.request.context = parsed_context;
        } else if (argument == "--selection-context") {
            const std::string token = require_value(argument);
            copperfin::studio::StudioEditorSelectionContext parsed_context{};
            if (!parse_editor_selection_context_token(token, parsed_context)) {
                fail(builder_parse_unknown_selection_context_token(catalog, token));
                continue;
            }
            result.selection_context_provided = true;
            result.selection_context = parsed_context;
        } else if (argument == "--path") {
            result.request.asset_path = require_value(argument);
        } else if (argument == "--record") {
            const std::string token = require_value(argument);
            std::size_t record_index = 0U;
            if (!parse_size_t_token(token, record_index)) {
                fail(builder_parse_record_non_negative_integer(catalog));
                continue;
            }
            result.request.record_index = record_index;
        } else if (argument == "--object-name") {
            result.request.object_name = require_value(argument);
        } else if (argument == "--unique-id") {
            result.request.unique_id = require_value(argument);
        } else if (argument == "--admit-ui-launch") {
            const std::string token = require_value(argument);
            bool admitted = false;
            if (!parse_bool_token(token, admitted)) {
                fail(builder_parse_boolean_value_required(catalog, "--admit-ui-launch"));
                continue;
            }
            result.admit_ui_launch = admitted;
        } else {
            fail(builder_parse_unknown_option(catalog, "builder-invocation-admission", argument));
        }
    }

    if (result.ok && result.request.builder_id.empty()) {
        fail(builder_parse_message(catalog, "StudioHost.BuilderParse.Error.NoBuilderId"));
    }
    if (result.ok && result.context_provided && result.selection_context_provided) {
        fail(builder_parse_context_conflict(catalog, "StudioHost.BuilderParse.RequestName.InvocationAdmission"));
    }
    if (result.ok && !result.context_provided && !result.selection_context_provided) {
        fail(builder_parse_message(catalog, "StudioHost.BuilderParse.Error.NoBuilderOrSelectionContext"));
    }
    return result;
}

BuilderInvocationAdmissionCatalogParseResult parse_builder_invocation_admission_catalog_arguments(
    const copperfin::localization::LocalizedCatalog& catalog,
    const std::vector<std::string>& args) {
    BuilderInvocationAdmissionCatalogParseResult result{};
    result.output_json = std::find(args.begin(), args.end(), "--json") != args.end();
    result.requested =
        std::find(args.begin(), args.end(), "--builder-invocation-admission-catalog") != args.end();
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
                fail(builder_parse_missing_value(catalog, option));
                return {};
            }
            ++index;
            return args[index];
        };

        if (argument == "--json" || argument == "--builder-invocation-admission-catalog") {
            continue;
        }
        if (argument == "--builder-context") {
            const std::string token = require_value(argument);
            copperfin::studio::StudioBuilderContext parsed_context{};
            if (!parse_builder_context_token(token, parsed_context)) {
                fail(builder_parse_unknown_builder_context_token(catalog, token));
                continue;
            }
            result.context_provided = true;
            result.request.context = parsed_context;
        } else if (argument == "--path") {
            result.request.asset_path = require_value(argument);
        } else if (argument == "--record") {
            const std::string token = require_value(argument);
            std::size_t record_index = 0U;
            if (!parse_size_t_token(token, record_index)) {
                fail(builder_parse_record_non_negative_integer(catalog));
                continue;
            }
            result.request.record_index = record_index;
        } else if (argument == "--object-name") {
            result.request.object_name = require_value(argument);
        } else if (argument == "--unique-id") {
            result.request.unique_id = require_value(argument);
        } else if (argument == "--admit-ui-launch") {
            const std::string token = require_value(argument);
            bool admitted = false;
            if (!parse_bool_token(token, admitted)) {
                fail(builder_parse_boolean_value_required(catalog, "--admit-ui-launch"));
                continue;
            }
            result.request.admit_ui_launches = admitted;
        } else {
            fail(builder_parse_unknown_option(catalog, "builder-invocation-admission-catalog", argument));
        }
    }

    if (result.ok && !result.context_provided) {
        fail(builder_parse_message(catalog, "StudioHost.BuilderParse.Error.NoBuilderContext"));
    }
    return result;
}

BuilderDispatchParseResult parse_builder_dispatch_arguments(
    const copperfin::localization::LocalizedCatalog& catalog,
    const std::vector<std::string>& args) {
    BuilderDispatchParseResult result{};
    result.output_json = std::find(args.begin(), args.end(), "--json") != args.end();
    result.requested = std::find(args.begin(), args.end(), "--builder-dispatch") != args.end();
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
                fail(builder_parse_missing_value(catalog, option));
                return {};
            }
            ++index;
            return args[index];
        };

        if (argument == "--json") {
            continue;
        }
        if (argument == "--builder-dispatch") {
            result.request.builder_id = require_value(argument);
        } else if (argument == "--builder-context") {
            const std::string token = require_value(argument);
            copperfin::studio::StudioBuilderContext parsed_context{};
            if (!parse_builder_context_token(token, parsed_context)) {
                fail(builder_parse_unknown_builder_context_token(catalog, token));
                continue;
            }
            result.context_provided = true;
            result.request.context = parsed_context;
        } else if (argument == "--selection-context") {
            const std::string token = require_value(argument);
            copperfin::studio::StudioEditorSelectionContext parsed_context{};
            if (!parse_editor_selection_context_token(token, parsed_context)) {
                fail(builder_parse_unknown_selection_context_token(catalog, token));
                continue;
            }
            result.selection_context_provided = true;
            result.selection_context = parsed_context;
        } else if (argument == "--path") {
            result.request.asset_path = require_value(argument);
        } else if (argument == "--record") {
            const std::string token = require_value(argument);
            std::size_t record_index = 0U;
            if (!parse_size_t_token(token, record_index)) {
                fail(builder_parse_record_non_negative_integer(catalog));
                continue;
            }
            result.request.record_index = record_index;
        } else if (argument == "--object-name") {
            result.request.object_name = require_value(argument);
        } else if (argument == "--unique-id") {
            result.request.unique_id = require_value(argument);
        } else if (argument == "--admit-ui-launch") {
            const std::string token = require_value(argument);
            bool admitted = false;
            if (!parse_bool_token(token, admitted)) {
                fail(builder_parse_boolean_value_required(catalog, "--admit-ui-launch"));
                continue;
            }
            result.admit_ui_launch = admitted;
        } else {
            fail(builder_parse_unknown_option(catalog, "builder-dispatch", argument));
        }
    }

    if (result.ok && result.request.builder_id.empty()) {
        fail(builder_parse_message(catalog, "StudioHost.BuilderParse.Error.NoBuilderId"));
    }
    if (result.ok && result.context_provided && result.selection_context_provided) {
        fail(builder_parse_context_conflict(catalog, "StudioHost.BuilderParse.RequestName.Dispatch"));
    }
    if (result.ok && !result.context_provided && !result.selection_context_provided) {
        fail(builder_parse_message(catalog, "StudioHost.BuilderParse.Error.NoBuilderOrSelectionContext"));
    }
    return result;
}

BuilderExecuteParseResult parse_builder_execute_arguments(
    const copperfin::localization::LocalizedCatalog& catalog,
    const std::vector<std::string>& args) {
    BuilderExecuteParseResult result{};
    result.output_json = std::find(args.begin(), args.end(), "--json") != args.end();
    result.requested = std::find(args.begin(), args.end(), "--builder-execute") != args.end();
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
                fail(builder_parse_missing_value(catalog, option));
                return {};
            }
            ++index;
            return args[index];
        };

        if (argument == "--json") {
            continue;
        }
        if (argument == "--builder-execute") {
            result.request.builder_id = require_value(argument);
        } else if (argument == "--builder-context") {
            const std::string token = require_value(argument);
            copperfin::studio::StudioBuilderContext parsed_context{};
            if (!parse_builder_context_token(token, parsed_context)) {
                fail(builder_parse_unknown_builder_context_token(catalog, token));
                continue;
            }
            result.context_provided = true;
            result.request.context = parsed_context;
        } else if (argument == "--selection-context") {
            const std::string token = require_value(argument);
            copperfin::studio::StudioEditorSelectionContext parsed_context{};
            if (!parse_editor_selection_context_token(token, parsed_context)) {
                fail(builder_parse_unknown_selection_context_token(catalog, token));
                continue;
            }
            result.selection_context_provided = true;
            result.selection_context = parsed_context;
        } else if (argument == "--path") {
            result.request.asset_path = require_value(argument);
        } else if (argument == "--record") {
            const std::string token = require_value(argument);
            std::size_t record_index = 0U;
            if (!parse_size_t_token(token, record_index)) {
                fail(builder_parse_record_non_negative_integer(catalog));
                continue;
            }
            result.request.record_index = record_index;
        } else if (argument == "--object-name") {
            result.request.object_name = require_value(argument);
        } else if (argument == "--unique-id") {
            result.request.unique_id = require_value(argument);
        } else if (argument == "--admit-ui-launch") {
            const std::string token = require_value(argument);
            bool admitted = false;
            if (!parse_bool_token(token, admitted)) {
                fail(builder_parse_boolean_value_required(catalog, "--admit-ui-launch"));
                continue;
            }
            result.admit_ui_launch = admitted;
        } else if (argument == "--admit-builder-execution") {
            const std::string token = require_value(argument);
            bool admitted = false;
            if (!parse_bool_token(token, admitted)) {
                fail(builder_parse_boolean_value_required(catalog, "--admit-builder-execution"));
                continue;
            }
            result.admit_execution = admitted;
        } else if (argument == "--builder-launch-command") {
            result.launch_command = require_value(argument);
        } else {
            fail(builder_parse_unknown_option(catalog, "builder-execute", argument));
        }
    }

    if (result.ok && result.request.builder_id.empty()) {
        fail(builder_parse_message(catalog, "StudioHost.BuilderParse.Error.NoBuilderId"));
    }
    if (result.ok && result.context_provided && result.selection_context_provided) {
        fail(builder_parse_context_conflict(catalog, "StudioHost.BuilderParse.RequestName.Execute"));
    }
    if (result.ok && !result.context_provided && !result.selection_context_provided) {
        fail(builder_parse_message(catalog, "StudioHost.BuilderParse.Error.NoBuilderOrSelectionContext"));
    }
    if (result.ok && result.launch_command.empty()) {
        fail(builder_parse_message(catalog, "StudioHost.BuilderParse.Error.NoBuilderLaunchCommand"));
    }
    return result;
}

BuilderDispatchCatalogParseResult parse_builder_dispatch_catalog_arguments(
    const copperfin::localization::LocalizedCatalog& catalog,
    const std::vector<std::string>& args) {
    BuilderDispatchCatalogParseResult result{};
    result.output_json = std::find(args.begin(), args.end(), "--json") != args.end();
    result.requested = std::find(args.begin(), args.end(), "--builder-dispatch-catalog") != args.end();
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
                fail(builder_parse_missing_value(catalog, option));
                return {};
            }
            ++index;
            return args[index];
        };

        if (argument == "--json" || argument == "--builder-dispatch-catalog") {
            continue;
        }
        if (argument == "--builder-context") {
            const std::string token = require_value(argument);
            copperfin::studio::StudioBuilderContext parsed_context{};
            if (!parse_builder_context_token(token, parsed_context)) {
                fail(builder_parse_unknown_builder_context_token(catalog, token));
                continue;
            }
            result.context_provided = true;
            result.request.context = parsed_context;
        } else if (argument == "--path") {
            result.request.asset_path = require_value(argument);
        } else if (argument == "--record") {
            const std::string token = require_value(argument);
            std::size_t record_index = 0U;
            if (!parse_size_t_token(token, record_index)) {
                fail(builder_parse_record_non_negative_integer(catalog));
                continue;
            }
            result.request.record_index = record_index;
        } else if (argument == "--object-name") {
            result.request.object_name = require_value(argument);
        } else if (argument == "--unique-id") {
            result.request.unique_id = require_value(argument);
        } else if (argument == "--admit-ui-launch") {
            const std::string token = require_value(argument);
            bool admitted = false;
            if (!parse_bool_token(token, admitted)) {
                fail(builder_parse_boolean_value_required(catalog, "--admit-ui-launch"));
                continue;
            }
            result.request.admit_ui_launches = admitted;
        } else {
            fail(builder_parse_unknown_option(catalog, "builder-dispatch-catalog", argument));
        }
    }

    if (result.ok && !result.context_provided) {
        fail(builder_parse_message(catalog, "StudioHost.BuilderParse.Error.NoBuilderContext"));
    }
    return result;
}

BuilderDispatchExecutionCatalogParseResult parse_builder_dispatch_execution_catalog_arguments(
    const copperfin::localization::LocalizedCatalog& catalog,
    const std::vector<std::string>& args) {
    BuilderDispatchExecutionCatalogParseResult result{};
    result.output_json = std::find(args.begin(), args.end(), "--json") != args.end();
    result.requested = std::find(args.begin(), args.end(), "--builder-dispatch-execution-catalog") != args.end();
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
                fail(builder_parse_missing_value(catalog, option));
                return {};
            }
            ++index;
            return args[index];
        };

        if (argument == "--json" || argument == "--builder-dispatch-execution-catalog") {
            continue;
        }
        if (argument == "--builder-context") {
            const std::string token = require_value(argument);
            copperfin::studio::StudioBuilderContext parsed_context{};
            if (!parse_builder_context_token(token, parsed_context)) {
                fail(builder_parse_unknown_builder_context_token(catalog, token));
                continue;
            }
            result.context_provided = true;
            result.request.context = parsed_context;
        } else if (argument == "--path") {
            result.request.asset_path = require_value(argument);
        } else if (argument == "--record") {
            const std::string token = require_value(argument);
            std::size_t record_index = 0U;
            if (!parse_size_t_token(token, record_index)) {
                fail(builder_parse_record_non_negative_integer(catalog));
                continue;
            }
            result.request.record_index = record_index;
        } else if (argument == "--object-name") {
            result.request.object_name = require_value(argument);
        } else if (argument == "--unique-id") {
            result.request.unique_id = require_value(argument);
        } else if (argument == "--admit-ui-launch") {
            const std::string token = require_value(argument);
            bool admitted = false;
            if (!parse_bool_token(token, admitted)) {
                fail(builder_parse_boolean_value_required(catalog, "--admit-ui-launch"));
                continue;
            }
            result.request.admit_ui_launches = admitted;
        } else if (argument == "--admit-builder-execution") {
            const std::string token = require_value(argument);
            bool admitted = false;
            if (!parse_bool_token(token, admitted)) {
                fail(builder_parse_boolean_value_required(catalog, "--admit-builder-execution"));
                continue;
            }
            result.request.admit_execution = admitted;
        } else {
            fail(builder_parse_unknown_option(catalog, "builder-dispatch-execution-catalog", argument));
        }
    }

    if (result.ok && !result.context_provided) {
        fail(builder_parse_message(catalog, "StudioHost.BuilderParse.Error.NoBuilderContext"));
    }
    return result;
}

EditorActionLaunchPlanParseResult parse_editor_action_launch_plan_arguments(
    const copperfin::localization::LocalizedCatalog& catalog,
    const std::vector<std::string>& args) {
    EditorActionLaunchPlanParseResult result{};
    result.output_json = std::find(args.begin(), args.end(), "--json") != args.end();
    result.requested = std::find(args.begin(), args.end(), "--editor-action-launch-plan") != args.end();
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
                fail(editor_action_parse_missing_value(catalog, option));
                return {};
            }
            ++index;
            return args[index];
        };

        if (argument == "--json") {
            continue;
        }
        if (argument == "--editor-action-launch-plan") {
            result.request.action_id = require_value(argument);
        } else if (argument == "--selection-context") {
            const std::string token = require_value(argument);
            copperfin::studio::StudioEditorSelectionContext parsed_context{};
            if (!parse_editor_selection_context_token(token, parsed_context)) {
                fail(editor_action_parse_unknown_selection_context_token(catalog, token));
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
                fail(editor_action_parse_non_negative_integer(catalog, "--record"));
                continue;
            }
            result.request.record_index = record_index;
        } else if (argument == "--object-name") {
            result.request.object_name = require_value(argument);
        } else if (argument == "--unique-id") {
            result.request.unique_id = require_value(argument);
        } else if (argument == "--symbol") {
            result.request.symbol = require_value(argument);
        } else if (argument == "--line") {
            const std::string token = require_value(argument);
            std::size_t line = 0U;
            if (!parse_size_t_token(token, line)) {
                fail(editor_action_parse_non_negative_integer(catalog, "--line"));
                continue;
            }
            result.request.line = line;
        } else if (argument == "--column") {
            const std::string token = require_value(argument);
            std::size_t column = 0U;
            if (!parse_size_t_token(token, column)) {
                fail(editor_action_parse_non_negative_integer(catalog, "--column"));
                continue;
            }
            result.request.column = column;
        } else {
            fail(editor_action_parse_unknown_option(catalog, "editor-action-launch-plan", argument));
        }
    }

    if (result.ok && result.request.action_id.empty()) {
        fail(editor_action_parse_message(catalog, "StudioHost.EditorActionParse.Error.NoEditorActionId"));
    }
    if (result.ok && !result.selection_context_provided) {
        fail(editor_action_parse_message(catalog, "StudioHost.EditorActionParse.Error.NoSelectionContext"));
    }
    return result;
}

EditorActionLaunchCatalogParseResult
parse_editor_action_launch_catalog_arguments(
    const copperfin::localization::LocalizedCatalog& catalog,
    const std::vector<std::string>& args) {
    EditorActionLaunchCatalogParseResult result{};
    result.output_json = std::find(args.begin(), args.end(), "--json") != args.end();
    result.requested = std::find(args.begin(), args.end(), "--editor-action-launch-catalog") != args.end();
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
                fail(editor_action_parse_missing_value(catalog, option));
                return {};
            }
            ++index;
            return args[index];
        };

        if (argument == "--json" || argument == "--editor-action-launch-catalog") {
            continue;
        }
        if (argument == "--selection-context") {
            const std::string token = require_value(argument);
            copperfin::studio::StudioEditorSelectionContext parsed_context{};
            if (!parse_editor_selection_context_token(token, parsed_context)) {
                fail(editor_action_parse_unknown_selection_context_token(catalog, token));
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
                fail(editor_action_parse_non_negative_integer(catalog, "--record"));
                continue;
            }
            result.request.record_index = record_index;
        } else if (argument == "--object-name") {
            result.request.object_name = require_value(argument);
        } else if (argument == "--unique-id") {
            result.request.unique_id = require_value(argument);
        } else if (argument == "--symbol") {
            result.request.symbol = require_value(argument);
        } else if (argument == "--line") {
            const std::string token = require_value(argument);
            std::size_t line = 0U;
            if (!parse_size_t_token(token, line)) {
                fail(editor_action_parse_non_negative_integer(catalog, "--line"));
                continue;
            }
            result.request.line = line;
        } else if (argument == "--column") {
            const std::string token = require_value(argument);
            std::size_t column = 0U;
            if (!parse_size_t_token(token, column)) {
                fail(editor_action_parse_non_negative_integer(catalog, "--column"));
                continue;
            }
            result.request.column = column;
        } else {
            fail(editor_action_parse_unknown_option(catalog, "editor-action-launch-catalog", argument));
        }
    }

    if (result.ok && !result.selection_context_provided) {
        fail(editor_action_parse_message(catalog, "StudioHost.EditorActionParse.Error.NoSelectionContext"));
    }
    return result;
}

EditorActionInvocationAdmissionParseResult parse_editor_action_invocation_admission_arguments(
    const copperfin::localization::LocalizedCatalog& catalog,
    const std::vector<std::string>& args) {
    EditorActionInvocationAdmissionParseResult result{};
    result.output_json = std::find(args.begin(), args.end(), "--json") != args.end();
    result.requested = std::find(args.begin(), args.end(), "--editor-action-invocation-admission") != args.end();
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
                fail(editor_action_parse_missing_value(catalog, option));
                return {};
            }
            ++index;
            return args[index];
        };

        if (argument == "--json") {
            continue;
        }
        if (argument == "--editor-action-invocation-admission") {
            result.request.action_id = require_value(argument);
        } else if (argument == "--selection-context") {
            const std::string token = require_value(argument);
            copperfin::studio::StudioEditorSelectionContext parsed_context{};
            if (!parse_editor_selection_context_token(token, parsed_context)) {
                fail(editor_action_parse_unknown_selection_context_token(catalog, token));
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
                fail(editor_action_parse_non_negative_integer(catalog, "--record"));
                continue;
            }
            result.request.record_index = record_index;
        } else if (argument == "--object-name") {
            result.request.object_name = require_value(argument);
        } else if (argument == "--unique-id") {
            result.request.unique_id = require_value(argument);
        } else if (argument == "--symbol") {
            result.request.symbol = require_value(argument);
        } else if (argument == "--line") {
            const std::string token = require_value(argument);
            std::size_t line = 0U;
            if (!parse_size_t_token(token, line)) {
                fail(editor_action_parse_non_negative_integer(catalog, "--line"));
                continue;
            }
            result.request.line = line;
        } else if (argument == "--column") {
            const std::string token = require_value(argument);
            std::size_t column = 0U;
            if (!parse_size_t_token(token, column)) {
                fail(editor_action_parse_non_negative_integer(catalog, "--column"));
                continue;
            }
            result.request.column = column;
        } else if (argument == "--admit-editor-invocation") {
            const std::string token = require_value(argument);
            bool admitted = false;
            if (!parse_bool_token(token, admitted)) {
                fail(editor_action_parse_boolean_value_required(catalog, "--admit-editor-invocation"));
                continue;
            }
            result.admit_editor_invocation = admitted;
        } else {
            fail(editor_action_parse_unknown_option(catalog, "editor-action-invocation-admission", argument));
        }
    }

    if (result.ok && result.request.action_id.empty()) {
        fail(editor_action_parse_message(catalog, "StudioHost.EditorActionParse.Error.NoEditorActionId"));
    }
    if (result.ok && !result.selection_context_provided) {
        fail(editor_action_parse_message(catalog, "StudioHost.EditorActionParse.Error.NoSelectionContext"));
    }
    return result;
}

EditorActionInvocationAdmissionCatalogParseResult
parse_editor_action_invocation_admission_catalog_arguments(
    const copperfin::localization::LocalizedCatalog& catalog,
    const std::vector<std::string>& args) {
    EditorActionInvocationAdmissionCatalogParseResult result{};
    result.output_json = std::find(args.begin(), args.end(), "--json") != args.end();
    result.requested =
        std::find(args.begin(), args.end(), "--editor-action-invocation-admission-catalog") != args.end();
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
                fail(editor_action_parse_missing_value(catalog, option));
                return {};
            }
            ++index;
            return args[index];
        };

        if (argument == "--json" || argument == "--editor-action-invocation-admission-catalog") {
            continue;
        }
        if (argument == "--selection-context") {
            const std::string token = require_value(argument);
            copperfin::studio::StudioEditorSelectionContext parsed_context{};
            if (!parse_editor_selection_context_token(token, parsed_context)) {
                fail(editor_action_parse_unknown_selection_context_token(catalog, token));
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
                fail(editor_action_parse_non_negative_integer(catalog, "--record"));
                continue;
            }
            result.request.record_index = record_index;
        } else if (argument == "--object-name") {
            result.request.object_name = require_value(argument);
        } else if (argument == "--unique-id") {
            result.request.unique_id = require_value(argument);
        } else if (argument == "--symbol") {
            result.request.symbol = require_value(argument);
        } else if (argument == "--line") {
            const std::string token = require_value(argument);
            std::size_t line = 0U;
            if (!parse_size_t_token(token, line)) {
                fail(editor_action_parse_non_negative_integer(catalog, "--line"));
                continue;
            }
            result.request.line = line;
        } else if (argument == "--column") {
            const std::string token = require_value(argument);
            std::size_t column = 0U;
            if (!parse_size_t_token(token, column)) {
                fail(editor_action_parse_non_negative_integer(catalog, "--column"));
                continue;
            }
            result.request.column = column;
        } else if (argument == "--admit-editor-invocation") {
            const std::string token = require_value(argument);
            bool admitted = false;
            if (!parse_bool_token(token, admitted)) {
                fail(editor_action_parse_boolean_value_required(catalog, "--admit-editor-invocation"));
                continue;
            }
            result.request.admit_editor_invocations = admitted;
        } else {
            fail(editor_action_parse_unknown_option(catalog, "editor-action-invocation-admission-catalog", argument));
        }
    }

    if (result.ok && !result.selection_context_provided) {
        fail(editor_action_parse_message(catalog, "StudioHost.EditorActionParse.Error.NoSelectionContext"));
    }
    return result;
}

EditorActionDispatchParseResult parse_editor_action_dispatch_arguments(
    const copperfin::localization::LocalizedCatalog& catalog,
    const std::vector<std::string>& args) {
    EditorActionDispatchParseResult result{};
    result.output_json = std::find(args.begin(), args.end(), "--json") != args.end();
    result.requested = std::find(args.begin(), args.end(), "--editor-action-dispatch") != args.end();
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
                fail(editor_action_parse_missing_value(catalog, option));
                return {};
            }
            ++index;
            return args[index];
        };

        if (argument == "--json") {
            continue;
        }
        if (argument == "--editor-action-dispatch") {
            result.request.action_id = require_value(argument);
        } else if (argument == "--selection-context") {
            const std::string token = require_value(argument);
            copperfin::studio::StudioEditorSelectionContext parsed_context{};
            if (!parse_editor_selection_context_token(token, parsed_context)) {
                fail(editor_action_parse_unknown_selection_context_token(catalog, token));
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
                fail(editor_action_parse_non_negative_integer(catalog, "--record"));
                continue;
            }
            result.request.record_index = record_index;
        } else if (argument == "--object-name") {
            result.request.object_name = require_value(argument);
        } else if (argument == "--unique-id") {
            result.request.unique_id = require_value(argument);
        } else if (argument == "--symbol") {
            result.request.symbol = require_value(argument);
        } else if (argument == "--line") {
            const std::string token = require_value(argument);
            std::size_t line = 0U;
            if (!parse_size_t_token(token, line)) {
                fail(editor_action_parse_non_negative_integer(catalog, "--line"));
                continue;
            }
            result.request.line = line;
        } else if (argument == "--column") {
            const std::string token = require_value(argument);
            std::size_t column = 0U;
            if (!parse_size_t_token(token, column)) {
                fail(editor_action_parse_non_negative_integer(catalog, "--column"));
                continue;
            }
            result.request.column = column;
        } else if (argument == "--admit-editor-invocation") {
            const std::string token = require_value(argument);
            bool admitted = false;
            if (!parse_bool_token(token, admitted)) {
                fail(editor_action_parse_boolean_value_required(catalog, "--admit-editor-invocation"));
                continue;
            }
            result.admit_editor_invocation = admitted;
        } else {
            fail(editor_action_parse_unknown_option(catalog, "editor-action-dispatch", argument));
        }
    }

    if (result.ok && result.request.action_id.empty()) {
        fail(editor_action_parse_message(catalog, "StudioHost.EditorActionParse.Error.NoEditorActionId"));
    }
    if (result.ok && !result.selection_context_provided) {
        fail(editor_action_parse_message(catalog, "StudioHost.EditorActionParse.Error.NoSelectionContext"));
    }
    return result;
}

EditorActionExecuteParseResult parse_editor_action_execute_arguments(
    const copperfin::localization::LocalizedCatalog& catalog,
    const std::vector<std::string>& args) {
    EditorActionExecuteParseResult result{};
    result.output_json = std::find(args.begin(), args.end(), "--json") != args.end();
    result.requested = std::find(args.begin(), args.end(), "--editor-action-execute") != args.end();
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
                fail(editor_action_parse_missing_value(catalog, option));
                return {};
            }
            ++index;
            return args[index];
        };

        if (argument == "--json") {
            continue;
        }
        if (argument == "--editor-action-execute") {
            result.request.action_id = require_value(argument);
        } else if (argument == "--selection-context") {
            const std::string token = require_value(argument);
            copperfin::studio::StudioEditorSelectionContext parsed_context{};
            if (!parse_editor_selection_context_token(token, parsed_context)) {
                fail(editor_action_parse_unknown_selection_context_token(catalog, token));
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
                fail(editor_action_parse_non_negative_integer(catalog, "--record"));
                continue;
            }
            result.request.record_index = record_index;
        } else if (argument == "--object-name") {
            result.request.object_name = require_value(argument);
        } else if (argument == "--unique-id") {
            result.request.unique_id = require_value(argument);
        } else if (argument == "--symbol") {
            result.request.symbol = require_value(argument);
        } else if (argument == "--line") {
            const std::string token = require_value(argument);
            std::size_t line = 0U;
            if (!parse_size_t_token(token, line)) {
                fail(editor_action_parse_non_negative_integer(catalog, "--line"));
                continue;
            }
            result.request.line = line;
        } else if (argument == "--column") {
            const std::string token = require_value(argument);
            std::size_t column = 0U;
            if (!parse_size_t_token(token, column)) {
                fail(editor_action_parse_non_negative_integer(catalog, "--column"));
                continue;
            }
            result.request.column = column;
        } else if (argument == "--admit-editor-invocation") {
            const std::string token = require_value(argument);
            bool admitted = false;
            if (!parse_bool_token(token, admitted)) {
                fail(editor_action_parse_boolean_value_required(catalog, "--admit-editor-invocation"));
                continue;
            }
            result.admit_editor_invocation = admitted;
        } else if (argument == "--admit-editor-action-execution") {
            const std::string token = require_value(argument);
            bool admitted = false;
            if (!parse_bool_token(token, admitted)) {
                fail(editor_action_parse_boolean_value_required(catalog, "--admit-editor-action-execution"));
                continue;
            }
            result.admit_execution = admitted;
        } else if (argument == "--editor-action-launch-command") {
            result.launch_command = require_value(argument);
        } else {
            fail(editor_action_parse_unknown_option(catalog, "editor-action-execute", argument));
        }
    }

    if (result.ok && result.request.action_id.empty()) {
        fail(editor_action_parse_message(catalog, "StudioHost.EditorActionParse.Error.NoEditorActionId"));
    }
    if (result.ok && !result.selection_context_provided) {
        fail(editor_action_parse_message(catalog, "StudioHost.EditorActionParse.Error.NoSelectionContext"));
    }
    if (result.ok && result.launch_command.empty()) {
        fail(editor_action_parse_message(catalog, "StudioHost.EditorActionParse.Error.NoEditorActionLaunchCommand"));
    }
    return result;
}

EditorActionDispatchCatalogParseResult parse_editor_action_dispatch_catalog_arguments(
    const copperfin::localization::LocalizedCatalog& catalog,
    const std::vector<std::string>& args) {
    EditorActionDispatchCatalogParseResult result{};
    result.output_json = std::find(args.begin(), args.end(), "--json") != args.end();
    result.requested = std::find(args.begin(), args.end(), "--editor-action-dispatch-catalog") != args.end();
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
                fail(editor_action_parse_missing_value(catalog, option));
                return {};
            }
            ++index;
            return args[index];
        };

        if (argument == "--json" || argument == "--editor-action-dispatch-catalog") {
            continue;
        }
        if (argument == "--selection-context") {
            const std::string token = require_value(argument);
            copperfin::studio::StudioEditorSelectionContext parsed_context{};
            if (!parse_editor_selection_context_token(token, parsed_context)) {
                fail(editor_action_parse_unknown_selection_context_token(catalog, token));
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
                fail(editor_action_parse_non_negative_integer(catalog, "--record"));
                continue;
            }
            result.request.record_index = record_index;
        } else if (argument == "--object-name") {
            result.request.object_name = require_value(argument);
        } else if (argument == "--unique-id") {
            result.request.unique_id = require_value(argument);
        } else if (argument == "--symbol") {
            result.request.symbol = require_value(argument);
        } else if (argument == "--line") {
            const std::string token = require_value(argument);
            std::size_t line = 0U;
            if (!parse_size_t_token(token, line)) {
                fail(editor_action_parse_non_negative_integer(catalog, "--line"));
                continue;
            }
            result.request.line = line;
        } else if (argument == "--column") {
            const std::string token = require_value(argument);
            std::size_t column = 0U;
            if (!parse_size_t_token(token, column)) {
                fail(editor_action_parse_non_negative_integer(catalog, "--column"));
                continue;
            }
            result.request.column = column;
        } else if (argument == "--admit-editor-invocation") {
            const std::string token = require_value(argument);
            bool admitted = false;
            if (!parse_bool_token(token, admitted)) {
                fail(editor_action_parse_boolean_value_required(catalog, "--admit-editor-invocation"));
                continue;
            }
            result.request.admit_editor_invocations = admitted;
        } else {
            fail(editor_action_parse_unknown_option(catalog, "editor-action-dispatch-catalog", argument));
        }
    }

    if (result.ok && !result.selection_context_provided) {
        fail(editor_action_parse_message(catalog, "StudioHost.EditorActionParse.Error.NoSelectionContext"));
    }
    return result;
}

EditorActionDispatchExecutionCatalogParseResult parse_editor_action_dispatch_execution_catalog_arguments(
    const copperfin::localization::LocalizedCatalog& catalog,
    const std::vector<std::string>& args) {
    EditorActionDispatchExecutionCatalogParseResult result{};
    result.output_json = std::find(args.begin(), args.end(), "--json") != args.end();
    result.requested =
        std::find(args.begin(), args.end(), "--editor-action-dispatch-execution-catalog") != args.end();
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
                fail(editor_action_parse_missing_value(catalog, option));
                return {};
            }
            ++index;
            return args[index];
        };

        if (argument == "--json" || argument == "--editor-action-dispatch-execution-catalog") {
            continue;
        }
        if (argument == "--selection-context") {
            const std::string token = require_value(argument);
            copperfin::studio::StudioEditorSelectionContext parsed_context{};
            if (!parse_editor_selection_context_token(token, parsed_context)) {
                fail(editor_action_parse_unknown_selection_context_token(catalog, token));
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
                fail(editor_action_parse_non_negative_integer(catalog, "--record"));
                continue;
            }
            result.request.record_index = record_index;
        } else if (argument == "--object-name") {
            result.request.object_name = require_value(argument);
        } else if (argument == "--unique-id") {
            result.request.unique_id = require_value(argument);
        } else if (argument == "--symbol") {
            result.request.symbol = require_value(argument);
        } else if (argument == "--line") {
            const std::string token = require_value(argument);
            std::size_t line = 0U;
            if (!parse_size_t_token(token, line)) {
                fail(editor_action_parse_non_negative_integer(catalog, "--line"));
                continue;
            }
            result.request.line = line;
        } else if (argument == "--column") {
            const std::string token = require_value(argument);
            std::size_t column = 0U;
            if (!parse_size_t_token(token, column)) {
                fail(editor_action_parse_non_negative_integer(catalog, "--column"));
                continue;
            }
            result.request.column = column;
        } else if (argument == "--admit-editor-invocation") {
            const std::string token = require_value(argument);
            bool admitted = false;
            if (!parse_bool_token(token, admitted)) {
                fail(editor_action_parse_boolean_value_required(catalog, "--admit-editor-invocation"));
                continue;
            }
            result.request.admit_editor_invocations = admitted;
        } else if (argument == "--admit-editor-action-execution") {
            const std::string token = require_value(argument);
            bool admitted = false;
            if (!parse_bool_token(token, admitted)) {
                fail(editor_action_parse_boolean_value_required(catalog, "--admit-editor-action-execution"));
                continue;
            }
            result.request.admit_execution = admitted;
        } else {
            fail(editor_action_parse_unknown_option(catalog, "editor-action-dispatch-execution-catalog", argument));
        }
    }

    if (result.ok && !result.selection_context_provided) {
        fail(editor_action_parse_message(catalog, "StudioHost.EditorActionParse.Error.NoSelectionContext"));
    }
    return result;
}

DesignerLaunchSurfacesParseResult parse_designer_launch_surfaces_arguments(
    const copperfin::localization::LocalizedCatalog& catalog,
    const std::vector<std::string>& args) {
    DesignerLaunchSurfacesParseResult result{};
    result.output_json = std::find(args.begin(), args.end(), "--json") != args.end();
    result.requested = std::find(args.begin(), args.end(), "--designer-launch-surfaces") != args.end();
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
                fail(designer_parse_missing_value(catalog, option));
                return {};
            }
            ++index;
            return args[index];
        };

        if (argument == "--json" || argument == "--designer-launch-surfaces") {
            continue;
        }
        if (argument == "--selection-context") {
            const std::string token = require_value(argument);
            copperfin::studio::StudioEditorSelectionContext parsed_context{};
            if (!parse_editor_selection_context_token(token, parsed_context)) {
                fail(designer_parse_unknown_selection_context_token(catalog, token));
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
                fail(designer_parse_non_negative_integer(catalog, "--record"));
                continue;
            }
            result.request.record_index = record_index;
        } else if (argument == "--object-name") {
            result.request.object_name = require_value(argument);
        } else if (argument == "--unique-id") {
            result.request.unique_id = require_value(argument);
        } else if (argument == "--symbol") {
            result.request.symbol = require_value(argument);
        } else if (argument == "--line") {
            const std::string token = require_value(argument);
            std::size_t line = 0U;
            if (!parse_size_t_token(token, line)) {
                fail(designer_parse_non_negative_integer(catalog, "--line"));
                continue;
            }
            result.request.line = line;
        } else if (argument == "--column") {
            const std::string token = require_value(argument);
            std::size_t column = 0U;
            if (!parse_size_t_token(token, column)) {
                fail(designer_parse_non_negative_integer(catalog, "--column"));
                continue;
            }
            result.request.column = column;
        } else {
            fail(designer_parse_unknown_option(catalog, "designer-launch-surfaces", argument));
        }
    }

    if (result.ok && !result.selection_context_provided) {
        fail(designer_parse_message(catalog, "StudioHost.DesignerParse.Error.NoSelectionContext"));
    }
    return result;
}

DesignerInvocationAdmissionParseResult parse_designer_invocation_admission_arguments(
    const copperfin::localization::LocalizedCatalog& catalog,
    const std::vector<std::string>& args) {
    DesignerInvocationAdmissionParseResult result{};
    result.output_json = std::find(args.begin(), args.end(), "--json") != args.end();
    result.requested = std::find(args.begin(), args.end(), "--designer-invocation-admission") != args.end();
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
                fail(designer_parse_missing_value(catalog, option));
                return {};
            }
            ++index;
            return args[index];
        };

        if (argument == "--json" || argument == "--designer-invocation-admission") {
            continue;
        }
        if (argument == "--selection-context") {
            const std::string token = require_value(argument);
            copperfin::studio::StudioEditorSelectionContext parsed_context{};
            if (!parse_editor_selection_context_token(token, parsed_context)) {
                fail(designer_parse_unknown_selection_context_token(catalog, token));
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
                fail(designer_parse_non_negative_integer(catalog, "--record"));
                continue;
            }
            result.request.record_index = record_index;
        } else if (argument == "--object-name") {
            result.request.object_name = require_value(argument);
        } else if (argument == "--unique-id") {
            result.request.unique_id = require_value(argument);
        } else if (argument == "--symbol") {
            result.request.symbol = require_value(argument);
        } else if (argument == "--line") {
            const std::string token = require_value(argument);
            std::size_t line = 0U;
            if (!parse_size_t_token(token, line)) {
                fail(designer_parse_non_negative_integer(catalog, "--line"));
                continue;
            }
            result.request.line = line;
        } else if (argument == "--column") {
            const std::string token = require_value(argument);
            std::size_t column = 0U;
            if (!parse_size_t_token(token, column)) {
                fail(designer_parse_non_negative_integer(catalog, "--column"));
                continue;
            }
            result.request.column = column;
        } else if (argument == "--admit-editor-invocations") {
            const std::string token = require_value(argument);
            bool admitted = false;
            if (!parse_bool_token(token, admitted)) {
                fail(designer_parse_boolean_value_required(catalog, "--admit-editor-invocations"));
                continue;
            }
            result.admit_editor_invocations = admitted;
        } else if (argument == "--admit-builder-invocations") {
            const std::string token = require_value(argument);
            bool admitted = false;
            if (!parse_bool_token(token, admitted)) {
                fail(designer_parse_boolean_value_required(catalog, "--admit-builder-invocations"));
                continue;
            }
            result.admit_builder_invocations = admitted;
        } else if (argument == "--admit-toolbox-invocation") {
            const std::string token = require_value(argument);
            bool admitted = false;
            if (!parse_bool_token(token, admitted)) {
                fail(designer_parse_boolean_value_required(catalog, "--admit-toolbox-invocation"));
                continue;
            }
            result.admit_toolbox_invocation = admitted;
        } else {
            fail(designer_parse_unknown_option(catalog, "designer-invocation-admission", argument));
        }
    }

    if (result.ok && !result.selection_context_provided) {
        fail(designer_parse_message(catalog, "StudioHost.DesignerParse.Error.NoSelectionContext"));
    }
    return result;
}

DesignerDispatchParseResult parse_designer_dispatch_arguments(
    const copperfin::localization::LocalizedCatalog& catalog,
    const std::vector<std::string>& args) {
    DesignerDispatchParseResult result{};
    result.output_json = std::find(args.begin(), args.end(), "--json") != args.end();
    result.requested = std::find(args.begin(), args.end(), "--designer-dispatch") != args.end();
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
                fail(designer_parse_missing_value(catalog, option));
                return {};
            }
            ++index;
            return args[index];
        };

        if (argument == "--json" || argument == "--designer-dispatch") {
            continue;
        }
        if (argument == "--selection-context") {
            const std::string token = require_value(argument);
            copperfin::studio::StudioEditorSelectionContext parsed_context{};
            if (!parse_editor_selection_context_token(token, parsed_context)) {
                fail(designer_parse_unknown_selection_context_token(catalog, token));
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
                fail(designer_parse_non_negative_integer(catalog, "--record"));
                continue;
            }
            result.request.record_index = record_index;
        } else if (argument == "--object-name") {
            result.request.object_name = require_value(argument);
        } else if (argument == "--unique-id") {
            result.request.unique_id = require_value(argument);
        } else if (argument == "--symbol") {
            result.request.symbol = require_value(argument);
        } else if (argument == "--line") {
            const std::string token = require_value(argument);
            std::size_t line = 0U;
            if (!parse_size_t_token(token, line)) {
                fail(designer_parse_non_negative_integer(catalog, "--line"));
                continue;
            }
            result.request.line = line;
        } else if (argument == "--column") {
            const std::string token = require_value(argument);
            std::size_t column = 0U;
            if (!parse_size_t_token(token, column)) {
                fail(designer_parse_non_negative_integer(catalog, "--column"));
                continue;
            }
            result.request.column = column;
        } else if (argument == "--admit-editor-invocations") {
            const std::string token = require_value(argument);
            bool admitted = false;
            if (!parse_bool_token(token, admitted)) {
                fail(designer_parse_boolean_value_required(catalog, "--admit-editor-invocations"));
                continue;
            }
            result.admit_editor_invocations = admitted;
        } else if (argument == "--admit-builder-invocations") {
            const std::string token = require_value(argument);
            bool admitted = false;
            if (!parse_bool_token(token, admitted)) {
                fail(designer_parse_boolean_value_required(catalog, "--admit-builder-invocations"));
                continue;
            }
            result.admit_builder_invocations = admitted;
        } else if (argument == "--admit-toolbox-invocation") {
            const std::string token = require_value(argument);
            bool admitted = false;
            if (!parse_bool_token(token, admitted)) {
                fail(designer_parse_boolean_value_required(catalog, "--admit-toolbox-invocation"));
                continue;
            }
            result.admit_toolbox_invocation = admitted;
        } else {
            fail(designer_parse_unknown_option(catalog, "designer-dispatch", argument));
        }
    }

    if (result.ok && !result.selection_context_provided) {
        fail(designer_parse_message(catalog, "StudioHost.DesignerParse.Error.NoSelectionContext"));
    }
    return result;
}

DesignerExecuteParseResult parse_designer_execute_arguments(
    const copperfin::localization::LocalizedCatalog& catalog,
    const std::vector<std::string>& args) {
    DesignerExecuteParseResult result{};
    result.output_json = std::find(args.begin(), args.end(), "--json") != args.end();
    result.requested = std::find(args.begin(), args.end(), "--designer-execute") != args.end();
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
                fail(designer_parse_missing_value(catalog, option));
                return {};
            }
            ++index;
            return args[index];
        };

        if (argument == "--json" || argument == "--designer-execute") {
            continue;
        }
        if (argument == "--selection-context") {
            const std::string token = require_value(argument);
            copperfin::studio::StudioEditorSelectionContext parsed_context{};
            if (!parse_editor_selection_context_token(token, parsed_context)) {
                fail(designer_parse_unknown_selection_context_token(catalog, token));
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
                fail(designer_parse_non_negative_integer(catalog, "--record"));
                continue;
            }
            result.request.record_index = record_index;
        } else if (argument == "--object-name") {
            result.request.object_name = require_value(argument);
        } else if (argument == "--unique-id") {
            result.request.unique_id = require_value(argument);
        } else if (argument == "--symbol") {
            result.request.symbol = require_value(argument);
        } else if (argument == "--line") {
            const std::string token = require_value(argument);
            std::size_t line = 0U;
            if (!parse_size_t_token(token, line)) {
                fail(designer_parse_non_negative_integer(catalog, "--line"));
                continue;
            }
            result.request.line = line;
        } else if (argument == "--column") {
            const std::string token = require_value(argument);
            std::size_t column = 0U;
            if (!parse_size_t_token(token, column)) {
                fail(designer_parse_non_negative_integer(catalog, "--column"));
                continue;
            }
            result.request.column = column;
        } else if (argument == "--admit-editor-invocations") {
            const std::string token = require_value(argument);
            bool admitted = false;
            if (!parse_bool_token(token, admitted)) {
                fail(designer_parse_boolean_value_required(catalog, "--admit-editor-invocations"));
                continue;
            }
            result.admit_editor_invocations = admitted;
        } else if (argument == "--admit-builder-invocations") {
            const std::string token = require_value(argument);
            bool admitted = false;
            if (!parse_bool_token(token, admitted)) {
                fail(designer_parse_boolean_value_required(catalog, "--admit-builder-invocations"));
                continue;
            }
            result.admit_builder_invocations = admitted;
        } else if (argument == "--admit-toolbox-invocation") {
            const std::string token = require_value(argument);
            bool admitted = false;
            if (!parse_bool_token(token, admitted)) {
                fail(designer_parse_boolean_value_required(catalog, "--admit-toolbox-invocation"));
                continue;
            }
            result.admit_toolbox_invocation = admitted;
        } else if (argument == "--admit-designer-execution") {
            const std::string token = require_value(argument);
            bool admitted = false;
            if (!parse_bool_token(token, admitted)) {
                fail(designer_parse_boolean_value_required(catalog, "--admit-designer-execution"));
                continue;
            }
            result.admit_execution = admitted;
        } else if (argument == "--editor-action-launch-command") {
            result.editor_action_launch_command = require_value(argument);
        } else if (argument == "--builder-launch-command") {
            result.builder_launch_command = require_value(argument);
        } else if (argument == "--toolbox-launch-command") {
            result.toolbox_launch_command = require_value(argument);
        } else {
            fail(designer_parse_unknown_option(catalog, "designer-execute", argument));
        }
    }

    if (result.ok && !result.selection_context_provided) {
        fail(designer_parse_message(catalog, "StudioHost.DesignerParse.Error.NoSelectionContext"));
    }
    if (result.ok && result.admit_editor_invocations && result.editor_action_launch_command.empty()) {
        fail(designer_parse_message(catalog, "StudioHost.DesignerParse.Error.NoDesignerEditorActionLaunchCommand"));
    }
    if (result.ok && result.admit_builder_invocations && result.builder_launch_command.empty()) {
        fail(designer_parse_message(catalog, "StudioHost.DesignerParse.Error.NoDesignerBuilderLaunchCommand"));
    }
    if (result.ok && result.admit_toolbox_invocation && result.toolbox_launch_command.empty()) {
        fail(designer_parse_message(catalog, "StudioHost.DesignerParse.Error.NoDesignerToolboxLaunchCommand"));
    }
    return result;
}

DesignerLaunchSurfaceCatalogParseResult parse_designer_launch_surface_catalog_arguments(
    const copperfin::localization::LocalizedCatalog& catalog,
    const std::vector<std::string>& args) {
    DesignerLaunchSurfaceCatalogParseResult result{};
    result.output_json = std::find(args.begin(), args.end(), "--json") != args.end();
    result.requested = std::find(args.begin(), args.end(), "--designer-launch-surface-catalog") != args.end();
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
                fail(designer_parse_missing_value(catalog, option));
                return {};
            }
            ++index;
            return args[index];
        };

        if (argument == "--json" || argument == "--designer-launch-surface-catalog") {
            continue;
        }
        if (argument == "--path") {
            result.request.asset_path = require_value(argument);
        } else if (argument == "--record") {
            const std::string token = require_value(argument);
            std::size_t record_index = 0U;
            if (!parse_size_t_token(token, record_index)) {
                fail(designer_parse_non_negative_integer(catalog, "--record"));
                continue;
            }
            result.request.record_index = record_index;
        } else if (argument == "--object-name") {
            result.request.object_name = require_value(argument);
        } else if (argument == "--unique-id") {
            result.request.unique_id = require_value(argument);
        } else if (argument == "--symbol") {
            result.request.symbol = require_value(argument);
        } else if (argument == "--line") {
            const std::string token = require_value(argument);
            std::size_t line = 0U;
            if (!parse_size_t_token(token, line)) {
                fail(designer_parse_non_negative_integer(catalog, "--line"));
                continue;
            }
            result.request.line = line;
        } else if (argument == "--column") {
            const std::string token = require_value(argument);
            std::size_t column = 0U;
            if (!parse_size_t_token(token, column)) {
                fail(designer_parse_non_negative_integer(catalog, "--column"));
                continue;
            }
            result.request.column = column;
        } else {
            fail(designer_parse_unknown_option(catalog, "designer-launch-surface-catalog", argument));
        }
    }
    return result;
}

DesignerInvocationAdmissionCatalogParseResult parse_designer_invocation_admission_catalog_arguments(
    const copperfin::localization::LocalizedCatalog& catalog,
    const std::vector<std::string>& args) {
    DesignerInvocationAdmissionCatalogParseResult result{};
    result.output_json = std::find(args.begin(), args.end(), "--json") != args.end();
    result.requested =
        std::find(args.begin(), args.end(), "--designer-invocation-admission-catalog") != args.end();
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
                fail(designer_parse_missing_value(catalog, option));
                return {};
            }
            ++index;
            return args[index];
        };

        if (argument == "--json" || argument == "--designer-invocation-admission-catalog") {
            continue;
        }
        if (argument == "--path") {
            result.request.asset_path = require_value(argument);
        } else if (argument == "--record") {
            const std::string token = require_value(argument);
            std::size_t record_index = 0U;
            if (!parse_size_t_token(token, record_index)) {
                fail(designer_parse_non_negative_integer(catalog, "--record"));
                continue;
            }
            result.request.record_index = record_index;
        } else if (argument == "--object-name") {
            result.request.object_name = require_value(argument);
        } else if (argument == "--unique-id") {
            result.request.unique_id = require_value(argument);
        } else if (argument == "--symbol") {
            result.request.symbol = require_value(argument);
        } else if (argument == "--line") {
            const std::string token = require_value(argument);
            std::size_t line = 0U;
            if (!parse_size_t_token(token, line)) {
                fail(designer_parse_non_negative_integer(catalog, "--line"));
                continue;
            }
            result.request.line = line;
        } else if (argument == "--column") {
            const std::string token = require_value(argument);
            std::size_t column = 0U;
            if (!parse_size_t_token(token, column)) {
                fail(designer_parse_non_negative_integer(catalog, "--column"));
                continue;
            }
            result.request.column = column;
        } else if (argument == "--admit-editor-invocations") {
            const std::string token = require_value(argument);
            bool admitted = false;
            if (!parse_bool_token(token, admitted)) {
                fail(designer_parse_boolean_value_required(catalog, "--admit-editor-invocations"));
                continue;
            }
            result.admit_editor_invocations = admitted;
            result.request.admit_editor_invocations = admitted;
        } else if (argument == "--admit-builder-invocations") {
            const std::string token = require_value(argument);
            bool admitted = false;
            if (!parse_bool_token(token, admitted)) {
                fail(designer_parse_boolean_value_required(catalog, "--admit-builder-invocations"));
                continue;
            }
            result.admit_builder_invocations = admitted;
            result.request.admit_builder_invocations = admitted;
        } else if (argument == "--admit-toolbox-invocation") {
            const std::string token = require_value(argument);
            bool admitted = false;
            if (!parse_bool_token(token, admitted)) {
                fail(designer_parse_boolean_value_required(catalog, "--admit-toolbox-invocation"));
                continue;
            }
            result.admit_toolbox_invocation = admitted;
            result.request.admit_toolbox_invocation = admitted;
        } else {
            fail(designer_parse_unknown_option(catalog, "designer-invocation-admission-catalog", argument));
        }
    }
    return result;
}

DesignerDispatchCatalogParseResult parse_designer_dispatch_catalog_arguments(
    const copperfin::localization::LocalizedCatalog& catalog,
    const std::vector<std::string>& args) {
    DesignerDispatchCatalogParseResult result{};
    result.output_json = std::find(args.begin(), args.end(), "--json") != args.end();
    result.requested =
        std::find(args.begin(), args.end(), "--designer-dispatch-catalog") != args.end();
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
                fail(designer_parse_missing_value(catalog, option));
                return {};
            }
            ++index;
            return args[index];
        };

        if (argument == "--json" || argument == "--designer-dispatch-catalog") {
            continue;
        }
        if (argument == "--path") {
            result.request.asset_path = require_value(argument);
        } else if (argument == "--record") {
            const std::string token = require_value(argument);
            std::size_t record_index = 0U;
            if (!parse_size_t_token(token, record_index)) {
                fail(designer_parse_non_negative_integer(catalog, "--record"));
                continue;
            }
            result.request.record_index = record_index;
        } else if (argument == "--object-name") {
            result.request.object_name = require_value(argument);
        } else if (argument == "--unique-id") {
            result.request.unique_id = require_value(argument);
        } else if (argument == "--symbol") {
            result.request.symbol = require_value(argument);
        } else if (argument == "--line") {
            const std::string token = require_value(argument);
            std::size_t line = 0U;
            if (!parse_size_t_token(token, line)) {
                fail(designer_parse_non_negative_integer(catalog, "--line"));
                continue;
            }
            result.request.line = line;
        } else if (argument == "--column") {
            const std::string token = require_value(argument);
            std::size_t column = 0U;
            if (!parse_size_t_token(token, column)) {
                fail(designer_parse_non_negative_integer(catalog, "--column"));
                continue;
            }
            result.request.column = column;
        } else if (argument == "--admit-editor-invocations") {
            const std::string token = require_value(argument);
            bool admitted = false;
            if (!parse_bool_token(token, admitted)) {
                fail(designer_parse_boolean_value_required(catalog, "--admit-editor-invocations"));
                continue;
            }
            result.admit_editor_invocations = admitted;
            result.request.admit_editor_invocations = admitted;
        } else if (argument == "--admit-builder-invocations") {
            const std::string token = require_value(argument);
            bool admitted = false;
            if (!parse_bool_token(token, admitted)) {
                fail(designer_parse_boolean_value_required(catalog, "--admit-builder-invocations"));
                continue;
            }
            result.admit_builder_invocations = admitted;
            result.request.admit_builder_invocations = admitted;
        } else if (argument == "--admit-toolbox-invocation") {
            const std::string token = require_value(argument);
            bool admitted = false;
            if (!parse_bool_token(token, admitted)) {
                fail(designer_parse_boolean_value_required(catalog, "--admit-toolbox-invocation"));
                continue;
            }
            result.admit_toolbox_invocation = admitted;
            result.request.admit_toolbox_invocation = admitted;
        } else {
            fail(designer_parse_unknown_option(catalog, "designer-dispatch-catalog", argument));
        }
    }
    return result;
}

DesignerDispatchExecutionCatalogParseResult parse_designer_dispatch_execution_catalog_arguments(
    const copperfin::localization::LocalizedCatalog& catalog,
    const std::vector<std::string>& args) {
    DesignerDispatchExecutionCatalogParseResult result{};
    result.output_json = std::find(args.begin(), args.end(), "--json") != args.end();
    result.requested =
        std::find(args.begin(), args.end(), "--designer-dispatch-execution-catalog") != args.end();
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
                fail(designer_parse_missing_value(catalog, option));
                return {};
            }
            ++index;
            return args[index];
        };

        if (argument == "--json" || argument == "--designer-dispatch-execution-catalog") {
            continue;
        }
        if (argument == "--path") {
            result.request.asset_path = require_value(argument);
        } else if (argument == "--record") {
            const std::string token = require_value(argument);
            std::size_t record_index = 0U;
            if (!parse_size_t_token(token, record_index)) {
                fail(designer_parse_non_negative_integer(catalog, "--record"));
                continue;
            }
            result.request.record_index = record_index;
        } else if (argument == "--object-name") {
            result.request.object_name = require_value(argument);
        } else if (argument == "--unique-id") {
            result.request.unique_id = require_value(argument);
        } else if (argument == "--symbol") {
            result.request.symbol = require_value(argument);
        } else if (argument == "--line") {
            const std::string token = require_value(argument);
            std::size_t line = 0U;
            if (!parse_size_t_token(token, line)) {
                fail(designer_parse_non_negative_integer(catalog, "--line"));
                continue;
            }
            result.request.line = line;
        } else if (argument == "--column") {
            const std::string token = require_value(argument);
            std::size_t column = 0U;
            if (!parse_size_t_token(token, column)) {
                fail(designer_parse_non_negative_integer(catalog, "--column"));
                continue;
            }
            result.request.column = column;
        } else if (argument == "--admit-editor-invocations") {
            const std::string token = require_value(argument);
            bool admitted = false;
            if (!parse_bool_token(token, admitted)) {
                fail(designer_parse_boolean_value_required(catalog, "--admit-editor-invocations"));
                continue;
            }
            result.admit_editor_invocations = admitted;
            result.request.admit_editor_invocations = admitted;
        } else if (argument == "--admit-builder-invocations") {
            const std::string token = require_value(argument);
            bool admitted = false;
            if (!parse_bool_token(token, admitted)) {
                fail(designer_parse_boolean_value_required(catalog, "--admit-builder-invocations"));
                continue;
            }
            result.admit_builder_invocations = admitted;
            result.request.admit_builder_invocations = admitted;
        } else if (argument == "--admit-toolbox-invocation") {
            const std::string token = require_value(argument);
            bool admitted = false;
            if (!parse_bool_token(token, admitted)) {
                fail(designer_parse_boolean_value_required(catalog, "--admit-toolbox-invocation"));
                continue;
            }
            result.admit_toolbox_invocation = admitted;
            result.request.admit_toolbox_invocation = admitted;
        } else if (argument == "--admit-designer-execution") {
            const std::string token = require_value(argument);
            bool admitted = false;
            if (!parse_bool_token(token, admitted)) {
                fail(designer_parse_boolean_value_required(catalog, "--admit-designer-execution"));
                continue;
            }
            result.admit_designer_execution = admitted;
            result.request.admit_execution = admitted;
        } else {
            fail(designer_parse_unknown_option(catalog, "designer-dispatch-execution-catalog", argument));
        }
    }
    return result;
}

void print_json_created_visual_object(
    const copperfin::vfp::VisualObjectCreatedObject& object,
    const std::string& indent) {
    std::cout << indent << "{\n";
    std::cout << indent << "  \"recordIndex\": " << object.record_index << ",\n";
    std::cout << indent << "  \"objectName\": ";
    print_json_string(object.object_name);
    std::cout << ",\n";
    std::cout << indent << "  \"uniqueId\": ";
    print_json_string(object.unique_id);
    std::cout << ",\n";
    std::cout << indent << "  \"parentName\": ";
    print_json_string(object.parent_name);
    std::cout << "\n";
    std::cout << indent << "}";
}

void print_json_line_index_or_null(std::size_t line_index) {
    if (line_index == copperfin::studio::StudioObjectMissingLineIndex) {
        std::cout << "null";
    } else {
        std::cout << line_index;
    }
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

void print_json_document(const copperfin::studio::StudioDocumentModel& document,
                         const copperfin::localization::LocalizedCatalog& catalog,
                         bool asset_mutation_performed) {
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
    const auto* selected_report_section = document.selection_record_available
        ? find_selected_report_section(report_layout, document.selection_record_index)
        : nullptr;
    const auto* selected_report_object = document.selection_record_available
        ? find_selected_report_object(report_layout, document.selection_record_index)
        : nullptr;
    const auto* selected_report_object_section = document.selection_record_available
        ? find_selected_report_object_section(report_layout, document.selection_record_index)
        : nullptr;
    const auto selected_report_settings = document.selection_record_available
        ? find_selected_report_settings(report_layout, document.selection_record_index)
        : std::vector<copperfin::studio::StudioNamedValue>{};
    const std::string selected_report_selection_kind = !selected_report_settings.empty()
        ? "settings"
        : selected_report_section != nullptr
            ? "section"
            : selected_report_object != nullptr
                ? "object"
                : "none";

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
        print_json_string(document.inspection.header.version_description(catalog));
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
    if (asset_mutation_performed) {
        std::cout << "    \"dryRun\": false,\n";
        std::cout << "    \"mutatesAsset\": true,\n";
        std::cout << "    \"undoAvailable\": "
                  << (command_undo_status.available ? "true" : "false") << ",\n";
        std::cout << "    \"undoLabel\": ";
        print_json_string(command_undo_status.label);
        std::cout << ",\n";
    }
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
        std::cout << "      \"documentTitleFieldIndex\": ";
        print_json_report_field_index_or_null(report_layout.document_title_field_index);
        std::cout << ",\n";
        std::cout << "      \"documentTitleMemoBlockNumber\": " << report_layout.document_title_memo_block_number << ",\n";
        std::cout << "      \"previewBoundsAvailable\": "
                  << (report_layout.preview_bounds_available ? "true" : "false") << ",\n";
        std::cout << "      \"previewBoundsLeft\": " << report_layout.preview_bounds_left << ",\n";
        std::cout << "      \"previewBoundsTop\": " << report_layout.preview_bounds_top << ",\n";
        std::cout << "      \"previewBoundsRight\": " << report_layout.preview_bounds_right << ",\n";
        std::cout << "      \"previewBoundsBottom\": " << report_layout.preview_bounds_bottom << ",\n";
        std::cout << "      \"previewBoundsWidth\": " << report_layout.preview_bounds_width << ",\n";
        std::cout << "      \"previewBoundsHeight\": " << report_layout.preview_bounds_height << ",\n";
        std::cout << "      \"deletedPreviewBoundsAvailable\": "
                  << (report_layout.deleted_preview_bounds_available ? "true" : "false") << ",\n";
        std::cout << "      \"deletedPreviewBoundsLeft\": " << report_layout.deleted_preview_bounds_left << ",\n";
        std::cout << "      \"deletedPreviewBoundsTop\": " << report_layout.deleted_preview_bounds_top << ",\n";
        std::cout << "      \"deletedPreviewBoundsRight\": " << report_layout.deleted_preview_bounds_right << ",\n";
        std::cout << "      \"deletedPreviewBoundsBottom\": " << report_layout.deleted_preview_bounds_bottom << ",\n";
        std::cout << "      \"deletedPreviewBoundsWidth\": " << report_layout.deleted_preview_bounds_width << ",\n";
        std::cout << "      \"deletedPreviewBoundsHeight\": " << report_layout.deleted_preview_bounds_height << ",\n";
        std::cout << "      \"pageSetupAvailable\": "
                  << (report_layout.page_setup_available ? "true" : "false") << ",\n";
        std::cout << "      \"orientationAvailable\": "
                  << (report_layout.orientation_available ? "true" : "false") << ",\n";
        std::cout << "      \"orientationCode\": " << report_layout.orientation_code << ",\n";
        std::cout << "      \"paperSizeAvailable\": "
                  << (report_layout.paper_size_available ? "true" : "false") << ",\n";
        std::cout << "      \"paperSizeCode\": " << report_layout.paper_size_code << ",\n";
        std::cout << "      \"paperLengthAvailable\": "
                  << (report_layout.paper_length_available ? "true" : "false") << ",\n";
        std::cout << "      \"paperLength\": " << report_layout.paper_length << ",\n";
        std::cout << "      \"paperWidthAvailable\": "
                  << (report_layout.paper_width_available ? "true" : "false") << ",\n";
        std::cout << "      \"paperWidth\": " << report_layout.paper_width << ",\n";
        std::cout << "      \"topMarginAvailable\": "
                  << (report_layout.top_margin_available ? "true" : "false") << ",\n";
        std::cout << "      \"topMargin\": " << report_layout.top_margin << ",\n";
        std::cout << "      \"bottomMarginAvailable\": "
                  << (report_layout.bottom_margin_available ? "true" : "false") << ",\n";
        std::cout << "      \"bottomMargin\": " << report_layout.bottom_margin << ",\n";
        std::cout << "      \"leftMarginAvailable\": "
                  << (report_layout.left_margin_available ? "true" : "false") << ",\n";
        std::cout << "      \"leftMargin\": " << report_layout.left_margin << ",\n";
        std::cout << "      \"rightMarginAvailable\": "
                  << (report_layout.right_margin_available ? "true" : "false") << ",\n";
        std::cout << "      \"rightMargin\": " << report_layout.right_margin << ",\n";
        std::cout << "      \"gridVerticalAvailable\": "
                  << (report_layout.grid_vertical_available ? "true" : "false") << ",\n";
        std::cout << "      \"gridVertical\": " << report_layout.grid_vertical << ",\n";
        std::cout << "      \"gridHorizontalAvailable\": "
                  << (report_layout.grid_horizontal_available ? "true" : "false") << ",\n";
        std::cout << "      \"gridHorizontal\": " << report_layout.grid_horizontal << ",\n";
        std::cout << "      \"colorAvailable\": "
                  << (report_layout.color_available ? "true" : "false") << ",\n";
        std::cout << "      \"color\": " << report_layout.color << ",\n";
        std::cout << "      \"copiesAvailable\": "
                  << (report_layout.copies_available ? "true" : "false") << ",\n";
        std::cout << "      \"copies\": " << report_layout.copies << ",\n";
        std::cout << "      \"driverAvailable\": "
                  << (report_layout.driver_available ? "true" : "false") << ",\n";
        std::cout << "      \"driver\": ";
        print_json_string(report_layout.driver);
        std::cout << ",\n";
        std::cout << "      \"deviceAvailable\": "
                  << (report_layout.device_available ? "true" : "false") << ",\n";
        std::cout << "      \"device\": ";
        print_json_string(report_layout.device);
        std::cout << ",\n";
        std::cout << "      \"outputAvailable\": "
                  << (report_layout.output_available ? "true" : "false") << ",\n";
        std::cout << "      \"output\": ";
        print_json_string(report_layout.output);
        std::cout << ",\n";
        std::cout << "      \"defaultSourceAvailable\": "
                  << (report_layout.default_source_available ? "true" : "false") << ",\n";
        std::cout << "      \"defaultSource\": " << report_layout.default_source << ",\n";
        std::cout << "      \"printQualityAvailable\": "
                  << (report_layout.print_quality_available ? "true" : "false") << ",\n";
        std::cout << "      \"printQuality\": " << report_layout.print_quality << ",\n";
        std::cout << "      \"yResolutionAvailable\": "
                  << (report_layout.y_resolution_available ? "true" : "false") << ",\n";
        std::cout << "      \"yResolution\": " << report_layout.y_resolution << ",\n";
        std::cout << "      \"trueTypeOptionAvailable\": "
                  << (report_layout.true_type_option_available ? "true" : "false") << ",\n";
        std::cout << "      \"trueTypeOption\": " << report_layout.true_type_option << ",\n";
        std::cout << "      \"asciiAvailable\": "
                  << (report_layout.ascii_available ? "true" : "false") << ",\n";
        std::cout << "      \"ascii\": " << report_layout.ascii << ",\n";
        std::cout << "      \"collateAvailable\": "
                  << (report_layout.collate_available ? "true" : "false") << ",\n";
        std::cout << "      \"collate\": " << report_layout.collate << ",\n";
        std::cout << "      \"columnSetupAvailable\": "
                  << (report_layout.column_setup_available ? "true" : "false") << ",\n";
        std::cout << "      \"columnCountAvailable\": "
                  << (report_layout.column_count_available ? "true" : "false") << ",\n";
        std::cout << "      \"columnCount\": " << report_layout.column_count << ",\n";
        std::cout << "      \"columnWidthAvailable\": "
                  << (report_layout.column_width_available ? "true" : "false") << ",\n";
        std::cout << "      \"columnWidth\": " << report_layout.column_width << ",\n";
        std::cout << "      \"columnSpacingAvailable\": "
                  << (report_layout.column_spacing_available ? "true" : "false") << ",\n";
        std::cout << "      \"columnSpacing\": " << report_layout.column_spacing << ",\n";
        std::cout << "      \"sortExpressionAvailable\": "
                  << (report_layout.sort_expression_available ? "true" : "false") << ",\n";
        std::cout << "      \"sortExpression\": ";
        print_json_string(report_layout.sort_expression);
        std::cout << ",\n";
        std::cout << "      \"liveObjectCount\": " << report_layout.live_object_count << ",\n";
        std::cout << "      \"placedObjectCount\": " << report_layout.placed_object_count << ",\n";
        std::cout << "      \"deletedPlacedObjectCount\": " << report_layout.deleted_placed_object_count << ",\n";
        std::cout << "      \"deletedUnplacedObjectCount\": " << report_layout.deleted_unplaced_object_count << ",\n";
        std::cout << "      \"objectKindCount\": " << report_layout.object_kind_counts.size() << ",\n";
        std::cout << "      \"unplacedObjectKindCount\": " << report_layout.unplaced_object_kind_counts.size() << ",\n";
        std::cout << "      \"deletedObjectKindCount\": " << report_layout.deleted_object_kind_counts.size() << ",\n";
        std::cout << "      \"sectionKindCount\": " << report_layout.section_kind_counts.size() << ",\n";
        std::cout << "      \"deletedSectionKindCount\": " << report_layout.deleted_section_kind_counts.size() << ",\n";
        std::cout << "      \"groupingCount\": " << report_layout.groupings.size() << ",\n";
        std::cout << "      \"sectionHeightTotal\": " << report_layout.section_height_total << ",\n";
        std::cout << "      \"deletedSectionHeightTotal\": " << report_layout.deleted_section_height_total << ",\n";
        std::cout << "      \"settingCount\": " << report_layout.settings.size() << ",\n";
        std::cout << "      \"deletedSettingCount\": " << report_layout.deleted_settings.size() << ",\n";
        std::cout << "      \"sectionCount\": " << report_layout.sections.size() << ",\n";
        std::cout << "      \"deletedSectionCount\": " << report_layout.deleted_sections.size() << ",\n";
        std::cout << "      \"unplacedObjectCount\": " << report_layout.unplaced_objects.size() << ",\n";
        std::cout << "      \"deletedObjectCount\": " << report_layout.deleted_objects.size() << ",\n";
        std::cout << "      \"objectKindCounts\": ";
        print_json_report_kind_counts(report_layout.object_kind_counts, "      ");
        std::cout << ",\n";
        std::cout << "      \"unplacedObjectKindCounts\": ";
        print_json_report_kind_counts(report_layout.unplaced_object_kind_counts, "      ");
        std::cout << ",\n";
        std::cout << "      \"deletedObjectKindCounts\": ";
        print_json_report_kind_counts(report_layout.deleted_object_kind_counts, "      ");
        std::cout << ",\n";
        std::cout << "      \"sectionKindCounts\": ";
        print_json_report_kind_counts(report_layout.section_kind_counts, "      ");
        std::cout << ",\n";
        std::cout << "      \"deletedSectionKindCounts\": ";
        print_json_report_kind_counts(report_layout.deleted_section_kind_counts, "      ");
        std::cout << ",\n";
        std::cout << "      \"groupings\": ";
        print_json_report_layout_groupings(report_layout.groupings, "      ");
        std::cout << ",\n";
        std::cout << "      \"settings\": ";
        print_json_report_named_values(report_layout.settings, "      ");
        std::cout << ",\n";
        std::cout << "      \"deletedSettings\": ";
        print_json_report_named_values(report_layout.deleted_settings, "      ");
        std::cout << ",\n";
        std::cout << "      \"sections\": ";
        print_json_report_layout_sections(report_layout.sections, "      ");
        std::cout << ",\n";
        std::cout << "      \"deletedSections\": ";
        print_json_report_layout_sections(report_layout.deleted_sections, "      ");
        std::cout << ",\n";
        std::cout << "      \"unplacedObjects\": ";
        print_json_report_layout_objects(report_layout.unplaced_objects, "      ");
        std::cout << ",\n";
        std::cout << "      \"deletedObjects\": ";
        print_json_report_layout_objects(report_layout.deleted_objects, "      ");
        std::cout << "\n";
        std::cout << "    },\n";
    }
    std::cout << "    \"selectedReportSectionAvailable\": "
              << (selected_report_section == nullptr ? "false" : "true") << ",\n";
    std::cout << "    \"selectedReportSection\": ";
    if (selected_report_section == nullptr) {
        std::cout << "null,\n";
    } else {
        print_json_report_layout_section(*selected_report_section, "    ");
        std::cout << ",\n";
    }
    std::cout << "    \"selectedReportObjectAvailable\": "
              << (selected_report_object == nullptr ? "false" : "true") << ",\n";
    std::cout << "    \"selectedReportObject\": ";
    if (selected_report_object == nullptr) {
        std::cout << "null,\n";
    } else {
        print_json_report_layout_object(*selected_report_object, "    ");
        std::cout << ",\n";
    }
    std::cout << "    \"selectedReportObjectSectionAvailable\": "
              << (selected_report_object_section == nullptr ? "false" : "true") << ",\n";
    std::cout << "    \"selectedReportObjectSection\": ";
    if (selected_report_object_section == nullptr) {
        std::cout << "null,\n";
    } else {
        print_json_report_layout_section(*selected_report_object_section, "    ");
        std::cout << ",\n";
    }
    std::cout << "    \"selectedReportSettingsAvailable\": "
              << (selected_report_settings.empty() ? "false" : "true") << ",\n";
    std::cout << "    \"selectedReportSettings\": ";
    if (selected_report_settings.empty()) {
        std::cout << "null,\n";
    } else {
        print_json_report_named_values(selected_report_settings, "    ");
        std::cout << ",\n";
    }
    std::cout << "    \"selectedReportSelectionAvailable\": "
              << (selected_report_selection_kind == "none" ? "false" : "true") << ",\n";
    std::cout << "    \"selectedReportSelectionKind\": ";
    print_json_string(selected_report_selection_kind);
    std::cout << ",\n";
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
    {
        const auto license_status = copperfin::licensing::load_license_status(g_executable_path);
        std::cout << "    \"licenseProfile\": {\n";
        std::cout << "      \"state\": ";
        print_json_string(std::string(copperfin::licensing::license_state_name(license_status.state)));
        std::cout << ",\n";
        std::cout << "      \"licenseType\": ";
        print_json_string(license_status.license_type);
        std::cout << ",\n";
        std::cout << "      \"licensee\": ";
        print_json_string(license_status.licensee_name);
        std::cout << ",\n";
        std::cout << "      \"seats\": " << license_status.seats << ",\n";
        std::cout << "      \"subscriptionExpires\": ";
        print_json_string(license_status.subscription_expires);
        std::cout << ",\n";
        std::cout << "      \"perpetualMaxMajorVersion\": " << license_status.perpetual_max_major_version << "\n";
        std::cout << "    },\n";
    }
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

void print_document(
    const copperfin::studio::StudioDocumentModel& document,
    const copperfin::localization::LocalizedCatalog& catalog) {
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
                  << document.inspection.header.version_description(catalog) << "\n";
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
            std::cout << " " << localized_message_or_default(
                "StudioHost.TablePreview.RecordDeleted",
                "deleted");
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

void print_json_subsystems(const copperfin::localization::LocalizedCatalog& catalog) {
    const auto subsystems = copperfin::studio::product_subsystems_for_catalog(catalog);
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
        std::cout << "      \"vfp9EquivalentDisplay\": ";
        print_json_string(std::string(subsystem.vfp9_equivalent_display));
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

void print_subsystems(const copperfin::localization::LocalizedCatalog& catalog) {
    const auto subsystems = copperfin::studio::product_subsystems_for_catalog(catalog);
    std::cout << "status: ok\n";
    std::cout << "subsystem_count: " << subsystems.size() << "\n";
    for (const auto& subsystem : subsystems) {
        std::cout << "subsystem.id: " << subsystem.id << "\n";
        std::cout << "  title: " << subsystem.title << "\n";
        std::cout << "  vfp9_equivalent: " << subsystem.vfp9_equivalent_display << "\n";
        std::cout << "  copperfin_component: " << subsystem.copperfin_component << "\n";
        std::cout << "  host_kind: " << copperfin::studio::product_host_kind_name(subsystem.host_kind) << "\n";
        std::cout << "  current_status: " << subsystem.current_status << "\n";
        std::cout << "  parity_scope: " << subsystem.parity_scope << "\n";
        std::cout << "  modern_editor_direction: " << subsystem.modern_editor_direction << "\n";
    }
}

std::optional<int> try_handle_list_subsystems(
    const copperfin::localization::LocalizedCatalog& catalog,
    const std::vector<std::string>& args) {
    const bool list_subsystems = std::find(args.begin(), args.end(), "--list-subsystems") != args.end();
    if (!(list_subsystems)) {
        return std::nullopt;
    }

        const bool output_json = std::find(args.begin(), args.end(), "--json") != args.end();
        if (output_json) {
            print_json_subsystems(catalog);
        } else {
            print_subsystems(catalog);
        }
        return 0;
    }

void print_license_status(
    const copperfin::licensing::LicenseStatus& status,
    const copperfin::localization::LocalizedCatalog& catalog) {
    using copperfin::licensing::LicenseState;

    std::cout << "status: ok\n";
    std::cout << "state: " << copperfin::licensing::license_state_name(status.state) << "\n";
    if (status.state == LicenseState::free) {
        return;
    }

    if (!status.license_id.empty()) {
        std::cout << "license_id: " << status.license_id << "\n";
    }
    if (!status.license_type.empty()) {
        std::cout << "license_type: " << status.license_type << "\n";
    }
    if (!status.pricing_model.empty()) {
        std::cout << "pricing_model: " << status.pricing_model << "\n";
    }
    if (!status.licensee_name.empty()) {
        std::cout << "licensee_name: " << status.licensee_name << "\n";
    }
    if (!status.licensee_email.empty()) {
        std::cout << "licensee_email: " << status.licensee_email << "\n";
    }
    if (status.seats > 0) {
        std::cout << "seats: " << status.seats << "\n";
    }
    if (!status.issued_date.empty()) {
        std::cout << "issued_date: " << status.issued_date << "\n";
    }
    if (!status.subscription_expires.empty()) {
        std::cout << "subscription_expires: " << status.subscription_expires << "\n";
    }
    if (status.perpetual_max_major_version > 0) {
        std::cout << "perpetual_max_major_version: " << status.perpetual_max_major_version << "\n";
    }
    if (!status.source_path.empty()) {
        std::cout << "source_path: " << status.source_path << "\n";
    }
    if (!status.diagnostic.empty()) {
        std::cout << "diagnostic: " << copperfin::licensing::localized_license_diagnostic(status, catalog) << "\n";
    }
}

void print_json_license_status(const copperfin::licensing::LicenseStatus& status) {
    std::cout << "{\n";
    std::cout << "  \"status\": \"ok\",\n";
    std::cout << "  \"license\": {\n";
    std::cout << "    \"state\": ";
    print_json_string(std::string(copperfin::licensing::license_state_name(status.state)));
    std::cout << ",\n";
    std::cout << "    \"licenseId\": ";
    print_json_string(status.license_id);
    std::cout << ",\n";
    std::cout << "    \"licenseType\": ";
    print_json_string(status.license_type);
    std::cout << ",\n";
    std::cout << "    \"pricingModel\": ";
    print_json_string(status.pricing_model);
    std::cout << ",\n";
    std::cout << "    \"licensee\": ";
    print_json_string(status.licensee_name);
    std::cout << ",\n";
    std::cout << "    \"seats\": " << status.seats << ",\n";
    std::cout << "    \"subscriptionExpires\": ";
    print_json_string(status.subscription_expires);
    std::cout << ",\n";
    std::cout << "    \"perpetualMaxMajorVersion\": " << status.perpetual_max_major_version << "\n";
    std::cout << "  }\n";
    std::cout << "}\n";
}

std::optional<int> try_handle_license_status(
    const copperfin::localization::LocalizedCatalog& catalog,
    const std::vector<std::string>& args) {
    (void)catalog;
    const bool requested = std::find(args.begin(), args.end(), "--license-status") != args.end();
    if (!requested) {
        return std::nullopt;
    }

    const auto status = copperfin::licensing::load_license_status(g_executable_path);
    const bool output_json = std::find(args.begin(), args.end(), "--json") != args.end();
    if (output_json) {
        print_json_license_status(status);
    } else {
        print_license_status(status, catalog);
    }
    return 0;
}

}  // namespace cf_studio_host_main_detail
