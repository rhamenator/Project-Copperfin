// Copyright © 2026 Richard M. Hamilton. All rights reserved.
// Licensed under the Project Copperfin Source-Available License or
// Commercial License. See LICENSE.md in the repository root.

#ifndef COPPERFIN_STUDIO_HOST_MAIN_SUPPORT_H
#define COPPERFIN_STUDIO_HOST_MAIN_SUPPORT_H

#include "copperfin/licensing/license_status.h"
#include "copperfin/localization/localization.h"
#include "copperfin/platform/path.h"
#include "copperfin/studio/document_model.h"
#include "copperfin/platform/database_model.h"
#include "copperfin/platform/extensibility_model.h"
#include "copperfin/security/process_hardening.h"
#include "copperfin/security/security_model.h"
#include "copperfin/studio/builder_dispatch.h"
#include "copperfin/studio/builder_invocation_admission.h"
#include "copperfin/studio/builder_registry.h"
#include "copperfin/studio/designer_dispatch.h"
#include "copperfin/studio/designer_invocation_admission.h"
#include "copperfin/studio/designer_launch_surfaces.h"
#include "copperfin/studio/editor_action_dispatch.h"
#include "copperfin/studio/editor_action_invocation_admission.h"
#include "copperfin/studio/project_workspace.h"
#include "copperfin/studio/product_subsystems.h"
#include "copperfin/studio/report_layout.h"
#include "copperfin/studio/toolbox_creation.h"
#include "copperfin/studio/toolbox_dispatch.h"
#include "copperfin/studio/toolbox_invocation_admission.h"
#include "copperfin/studio/toolbox_palette.h"
#include "copperfin/studio/vs_launch_contract.h"
#include "copperfin/runtime/rushmore_planning.h"
#include "copperfin/vfp/visual_asset_editor.h"

#include <algorithm>
#include <cctype>
#include <cstdlib>
#include <filesystem>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

#if !defined(_WIN32)
#include <sys/wait.h>
#endif
#include <optional>



namespace cf_studio_host_main_detail {

struct ToolboxCreateParseResult;
struct ToolboxCreatePlanParseResult;
struct SelectionToolboxCreatePlanParseResult;
struct SelectionToolboxCreateParseResult;
struct ToolboxCreateFromDispatchPlanParseResult;
struct ToolboxCreateFromDispatchParseResult;
struct ToolboxCreateDispatchPlanParseResult;
struct SelectionToolboxCreateDispatchPlanParseResult;
struct ToolboxCreateDispatchFromDispatchPlanParseResult;
struct ToolboxCreateBatchPlanParseResult;
struct SelectionToolboxCreateBatchPlanParseResult;
struct SelectionToolboxCreateBatchParseResult;
struct ToolboxCreateBatchFromDispatchPlanParseResult;
struct ToolboxCreateBatchFromDispatchParseResult;
struct ToolboxCreateBatchDispatchPlanParseResult;
struct SelectionToolboxCreateBatchDispatchPlanParseResult;
struct ToolboxCreateBatchDispatchFromDispatchPlanParseResult;
struct ToolboxCreateBatchParseResult;
struct ToolboxCreatePlanCatalogParseResult;
struct SelectionToolboxCreatePlanCatalogParseResult;
struct ToolboxCreateBatchPlanCatalogParseResult;
struct SelectionToolboxCreateBatchPlanCatalogParseResult;
struct ToolboxCreateDispatchCatalogParseResult;
struct SelectionToolboxCreateDispatchCatalogParseResult;
struct ToolboxCreateBatchDispatchCatalogParseResult;
struct SelectionToolboxCreateBatchDispatchCatalogParseResult;
struct BuilderLaunchPlanParseResult;
struct BuilderLaunchCatalogParseResult;
struct SelectionBuilderLaunchCatalogParseResult;
struct BuilderInvocationAdmissionParseResult;
struct BuilderInvocationAdmissionCatalogParseResult;
struct SelectionBuilderInvocationAdmissionCatalogParseResult;
struct BuilderDispatchParseResult;
struct BuilderExecuteParseResult;
struct BuilderDispatchCatalogParseResult;
struct BuilderDispatchExecutionCatalogParseResult;
struct SelectionBuilderDispatchCatalogParseResult;
struct SelectionBuilderDispatchExecutionCatalogParseResult;
struct EditorActionLaunchPlanParseResult;
struct EditorActionLaunchCatalogParseResult;
struct EditorActionInvocationAdmissionParseResult;
struct EditorActionInvocationAdmissionCatalogParseResult;
struct EditorActionDispatchParseResult;
struct EditorActionExecuteParseResult;
struct EditorActionDispatchCatalogParseResult;
struct EditorActionDispatchExecutionCatalogParseResult;
struct ToolboxPaletteLaunchPlanParseResult;
struct ToolboxPaletteLaunchCatalogParseResult;
struct ToolboxPaletteQueryParseResult;
struct VisualPropertyFilterParseResult;
struct VisualPropertyQueryParseResult;
struct VisualPropertyUpdateBatchParseResult;
struct VisualPropertyClearParseResult;
struct VisualPropertyClearBatchParseResult;
struct VisualPropertyCopyParseResult;
struct VisualPropertyCopyBatchParseResult;
struct VisualPropertyMoveParseResult;
struct VisualPropertyMoveBatchParseResult;
struct VisualPropertyRenameParseResult;
struct VisualPropertyRenameBatchParseResult;
struct VisualPropertyReorderParseResult;
struct VisualPropertyReorderBatchParseResult;
struct VisualPropertyListParseResult;
struct VisualObjectListParseResult;
struct VisualObjectChildrenParseResult;
struct VisualObjectDescendantsParseResult;
struct VisualObjectAncestorsParseResult;
struct VisualObjectReparentBatchParseResult;
struct VisualObjectDuplicateBatchParseResult;
struct VisualObjectDuplicateSubtreeParseResult;
struct VisualObjectRenameBatchParseResult;
struct VisualObjectReorderBatchParseResult;
struct VisualObjectUpdateBatchParseResult;
struct VisualMethodListParseResult;
struct VisualMethodQueryParseResult;
struct VisualMethodUpdateParseResult;
struct VisualMethodDeleteParseResult;
struct VisualMethodDeleteBatchParseResult;
struct VisualMethodRenameParseResult;
struct VisualMethodRenameBatchParseResult;
struct VisualMethodCopyParseResult;
struct VisualMethodCopyBatchParseResult;
struct VisualMethodMoveBatchParseResult;
struct VisualMethodMoveParseResult;
struct VisualMethodReorderParseResult;
struct VisualMethodReorderBatchParseResult;
struct ToolboxInvocationAdmissionParseResult;
struct ToolboxInvocationAdmissionCatalogParseResult;
struct SelectionToolboxInvocationAdmissionCatalogParseResult;
struct ToolboxDispatchParseResult;
struct ToolboxExecuteParseResult;
struct ToolboxDispatchCatalogParseResult;
struct ToolboxDispatchExecutionCatalogParseResult;
struct SelectionToolboxDispatchExecutionCatalogParseResult;
struct SelectionToolboxDispatchCatalogParseResult;
struct DesignerLaunchSurfacesParseResult;
struct DesignerInvocationAdmissionParseResult;
struct DesignerDispatchParseResult;
struct DesignerExecuteParseResult;
struct DesignerLaunchSurfaceCatalogParseResult;
struct DesignerInvocationAdmissionCatalogParseResult;
struct DesignerDispatchCatalogParseResult;
struct DesignerDispatchExecutionCatalogParseResult;
struct RushmoreExplainParseResult;

// ==== Shared CLI infrastructure (usage/help text, JSON scalar printing, localization, exit codes) ====

extern const copperfin::localization::LocalizedCatalog* g_active_catalog;
extern std::string g_executable_path;
copperfin::localization::LocalizedCatalog load_localization(std::string_view executable_path);
std::optional<int> try_handle_license_status(
    const copperfin::localization::LocalizedCatalog& catalog,
    const std::vector<std::string>& args);
std::string localized_message_or_default(
    std::string_view key,
    std::string_view fallback);
std::string studio_error_prefix();
std::string studio_warning_prefix();
void print_primary_usage_line(
    const copperfin::localization::LocalizedCatalog& catalog,
    std::string_view usage_template);
void print_alternate_usage_line(
    const copperfin::localization::LocalizedCatalog& catalog,
    std::string_view usage_template);
void print_object_usage_line(
    const copperfin::localization::LocalizedCatalog& catalog,
    std::string_view object_name,
    std::string_view usage_template);
void print_localized_object_usage_line(
    const copperfin::localization::LocalizedCatalog& catalog,
    std::string_view object_name_key,
    std::string_view usage_template);
void print_selection_context_tokens_line(
    const copperfin::localization::LocalizedCatalog& catalog,
    std::string_view selection_context_tokens);
void print_usage(const copperfin::localization::LocalizedCatalog& catalog);
std::string json_escape(const std::string& value);
void print_json_string(const std::string& value);
void print_json_string_view(std::string_view value);
void print_json_string_array(const std::vector<std::string>& values);
void print_json_int_array(const std::vector<int>& values);
std::string shell_quote(const std::string& value);
std::string build_shell_command(const std::string& launch_command, const std::vector<std::string>& arguments);
int execute_launch_command(const std::string& launch_command, const std::vector<std::string>& arguments);
bool parse_size_t_token(const std::string& token, std::size_t& value);
std::string lowercase_copy(std::string text);
RushmoreExplainParseResult parse_rushmore_explain_arguments(
    const copperfin::localization::LocalizedCatalog& catalog,
    const std::vector<std::string>& args);
void print_json_rushmore_explain_result(
    const copperfin::localization::LocalizedCatalog& catalog,
    const RushmoreExplainParseResult& result);
void print_rushmore_explain_result(
    const copperfin::localization::LocalizedCatalog& catalog,
    const RushmoreExplainParseResult& result);
std::optional<int> try_handle_rushmore_explain(
    const copperfin::localization::LocalizedCatalog& catalog,
    const std::vector<std::string>& args);
bool parse_bool_token(std::string token, bool& value);
bool parse_builder_context_token(
    const std::string& token,
    copperfin::studio::StudioBuilderContext& context);
bool parse_editor_selection_context_token(
    const std::string& token,
    copperfin::studio::StudioEditorSelectionContext& context);
BuilderLaunchPlanParseResult parse_builder_launch_plan_arguments(
    const copperfin::localization::LocalizedCatalog& catalog,
    const std::vector<std::string>& args);
BuilderLaunchCatalogParseResult parse_builder_launch_catalog_arguments(
    const copperfin::localization::LocalizedCatalog& catalog,
    const std::vector<std::string>& args);
BuilderInvocationAdmissionParseResult parse_builder_invocation_admission_arguments(
    const copperfin::localization::LocalizedCatalog& catalog,
    const std::vector<std::string>& args);
BuilderInvocationAdmissionCatalogParseResult parse_builder_invocation_admission_catalog_arguments(
    const copperfin::localization::LocalizedCatalog& catalog,
    const std::vector<std::string>& args);
BuilderDispatchParseResult parse_builder_dispatch_arguments(
    const copperfin::localization::LocalizedCatalog& catalog,
    const std::vector<std::string>& args);
BuilderExecuteParseResult parse_builder_execute_arguments(
    const copperfin::localization::LocalizedCatalog& catalog,
    const std::vector<std::string>& args);
BuilderDispatchCatalogParseResult parse_builder_dispatch_catalog_arguments(
    const copperfin::localization::LocalizedCatalog& catalog,
    const std::vector<std::string>& args);
BuilderDispatchExecutionCatalogParseResult parse_builder_dispatch_execution_catalog_arguments(
    const copperfin::localization::LocalizedCatalog& catalog,
    const std::vector<std::string>& args);
EditorActionLaunchPlanParseResult parse_editor_action_launch_plan_arguments(
    const copperfin::localization::LocalizedCatalog& catalog,
    const std::vector<std::string>& args);
EditorActionLaunchCatalogParseResult
parse_editor_action_launch_catalog_arguments(
    const copperfin::localization::LocalizedCatalog& catalog,
    const std::vector<std::string>& args);
EditorActionInvocationAdmissionParseResult parse_editor_action_invocation_admission_arguments(
    const copperfin::localization::LocalizedCatalog& catalog,
    const std::vector<std::string>& args);
EditorActionInvocationAdmissionCatalogParseResult
parse_editor_action_invocation_admission_catalog_arguments(
    const copperfin::localization::LocalizedCatalog& catalog,
    const std::vector<std::string>& args);
EditorActionDispatchParseResult parse_editor_action_dispatch_arguments(
    const copperfin::localization::LocalizedCatalog& catalog,
    const std::vector<std::string>& args);
EditorActionExecuteParseResult parse_editor_action_execute_arguments(
    const copperfin::localization::LocalizedCatalog& catalog,
    const std::vector<std::string>& args);
EditorActionDispatchCatalogParseResult parse_editor_action_dispatch_catalog_arguments(
    const copperfin::localization::LocalizedCatalog& catalog,
    const std::vector<std::string>& args);
EditorActionDispatchExecutionCatalogParseResult parse_editor_action_dispatch_execution_catalog_arguments(
    const copperfin::localization::LocalizedCatalog& catalog,
    const std::vector<std::string>& args);
DesignerLaunchSurfacesParseResult parse_designer_launch_surfaces_arguments(
    const copperfin::localization::LocalizedCatalog& catalog,
    const std::vector<std::string>& args);
DesignerInvocationAdmissionParseResult parse_designer_invocation_admission_arguments(
    const copperfin::localization::LocalizedCatalog& catalog,
    const std::vector<std::string>& args);
DesignerDispatchParseResult parse_designer_dispatch_arguments(
    const copperfin::localization::LocalizedCatalog& catalog,
    const std::vector<std::string>& args);
DesignerExecuteParseResult parse_designer_execute_arguments(
    const copperfin::localization::LocalizedCatalog& catalog,
    const std::vector<std::string>& args);
DesignerLaunchSurfaceCatalogParseResult parse_designer_launch_surface_catalog_arguments(
    const copperfin::localization::LocalizedCatalog& catalog,
    const std::vector<std::string>& args);
DesignerInvocationAdmissionCatalogParseResult parse_designer_invocation_admission_catalog_arguments(
    const copperfin::localization::LocalizedCatalog& catalog,
    const std::vector<std::string>& args);
DesignerDispatchCatalogParseResult parse_designer_dispatch_catalog_arguments(
    const copperfin::localization::LocalizedCatalog& catalog,
    const std::vector<std::string>& args);
DesignerDispatchExecutionCatalogParseResult parse_designer_dispatch_execution_catalog_arguments(
    const copperfin::localization::LocalizedCatalog& catalog,
    const std::vector<std::string>& args);
void print_json_created_visual_object(
    const copperfin::vfp::VisualObjectCreatedObject& object,
    const std::string& indent);
void print_json_line_index_or_null(std::size_t line_index);
void print_json_object_properties(
    const std::vector<copperfin::studio::StudioPropertySnapshot>& properties,
    const std::string& indent);
void print_json_record_index_or_null(std::size_t record_index);
void print_json_record_index_array(const std::vector<std::size_t>& record_indexes);
void print_json_object_summary(const copperfin::studio::StudioObjectSnapshot& object, const std::string& indent);
void print_json_document(const copperfin::studio::StudioDocumentModel& document,
                         const copperfin::localization::LocalizedCatalog& catalog,
                         bool asset_mutation_performed = false);
void print_document(
    const copperfin::studio::StudioDocumentModel& document,
    const copperfin::localization::LocalizedCatalog& catalog);
void print_json_subsystems(const copperfin::localization::LocalizedCatalog& catalog);
void print_subsystems(const copperfin::localization::LocalizedCatalog& catalog);
std::optional<int> try_handle_list_subsystems(
    const copperfin::localization::LocalizedCatalog& catalog,
    const std::vector<std::string>& args);

struct RushmoreExplainParseResult {
    bool requested = false;
    bool ok = true;
    bool output_json = false;
    std::string error;
    copperfin::runtime::RushmoreExplainPlan plan{};
};
struct ToolboxCreateParseResult {
    bool requested = false;
    bool ok = true;
    bool output_json = false;
    std::string error;
    copperfin::studio::StudioToolboxObjectCreateRequest request;
};
struct ToolboxCreatePlanParseResult {
    bool requested = false;
    bool ok = true;
    bool output_json = false;
    std::string error;
    copperfin::studio::StudioToolboxObjectCreateRequest request;
};
struct SelectionToolboxCreatePlanParseResult {
    bool requested = false;
    bool ok = true;
    bool output_json = false;
    bool selection_context_provided = false;
    std::string error;
    copperfin::studio::StudioSelectionToolboxObjectCreatePlanRequest request;
};
struct SelectionToolboxCreateParseResult {
    bool requested = false;
    bool ok = true;
    bool output_json = false;
    bool selection_context_provided = false;
    std::string error;
    copperfin::studio::StudioSelectionToolboxObjectCreatePlanRequest request;
};
struct ToolboxCreateFromDispatchPlanParseResult {
    bool requested = false;
    bool ok = true;
    bool output_json = false;
    bool selection_context_provided = false;
    bool admit_palette_invocation = false;
    std::string error;
    copperfin::studio::StudioToolboxPaletteLaunchRequest launch_request;
    copperfin::studio::StudioToolboxObjectCreateFromPaletteDispatchRequest create_request;
};
struct ToolboxCreateFromDispatchParseResult {
    bool requested = false;
    bool ok = true;
    bool output_json = false;
    bool selection_context_provided = false;
    bool admit_palette_invocation = false;
    std::string error;
    copperfin::studio::StudioToolboxPaletteLaunchRequest launch_request;
    copperfin::studio::StudioToolboxObjectCreateFromPaletteDispatchRequest create_request;
};
struct ToolboxCreateDispatchPlanParseResult {
    bool requested = false;
    bool ok = true;
    bool output_json = false;
    bool admit_create_operation = false;
    std::string error;
    copperfin::studio::StudioToolboxObjectCreateRequest request;
};
struct SelectionToolboxCreateDispatchPlanParseResult {
    bool requested = false;
    bool ok = true;
    bool output_json = false;
    bool selection_context_provided = false;
    std::string error;
    copperfin::studio::StudioSelectionToolboxObjectCreateDispatchRequest request;
};
struct ToolboxCreateDispatchFromDispatchPlanParseResult {
    bool requested = false;
    bool ok = true;
    bool output_json = false;
    bool selection_context_provided = false;
    bool admit_palette_invocation = false;
    std::string error;
    copperfin::studio::StudioToolboxPaletteLaunchRequest launch_request;
    copperfin::studio::StudioToolboxObjectCreateDispatchFromPaletteDispatchRequest dispatch_request;
};
struct ToolboxCreateBatchPlanParseResult {
    bool requested = false;
    bool ok = true;
    bool output_json = false;
    std::string error;
    copperfin::studio::StudioToolboxObjectCreateBatchPlanRequest request;
};
struct SelectionToolboxCreateBatchPlanParseResult {
    bool requested = false;
    bool ok = true;
    bool output_json = false;
    bool selection_context_provided = false;
    std::string error;
    copperfin::studio::StudioSelectionToolboxObjectCreateBatchPlanRequest request;
};
struct SelectionToolboxCreateBatchParseResult {
    bool requested = false;
    bool ok = true;
    bool output_json = false;
    bool selection_context_provided = false;
    std::string error;
    copperfin::studio::StudioSelectionToolboxObjectCreateBatchPlanRequest request;
};
struct ToolboxCreateBatchFromDispatchPlanParseResult {
    bool requested = false;
    bool ok = true;
    bool output_json = false;
    bool selection_context_provided = false;
    bool admit_palette_invocation = false;
    std::string error;
    copperfin::studio::StudioToolboxPaletteLaunchRequest launch_request;
    copperfin::studio::StudioToolboxObjectCreateBatchFromPaletteDispatchRequest create_request;
};
struct ToolboxCreateBatchFromDispatchParseResult {
    bool requested = false;
    bool ok = true;
    bool output_json = false;
    bool selection_context_provided = false;
    bool admit_palette_invocation = false;
    std::string error;
    copperfin::studio::StudioToolboxPaletteLaunchRequest launch_request;
    copperfin::studio::StudioToolboxObjectCreateBatchFromPaletteDispatchRequest create_request;
};
struct ToolboxCreateBatchDispatchPlanParseResult {
    bool requested = false;
    bool ok = true;
    bool output_json = false;
    bool admit_create_operation = false;
    std::string error;
    copperfin::studio::StudioToolboxObjectCreateBatchPlanRequest request;
};
struct SelectionToolboxCreateBatchDispatchPlanParseResult {
    bool requested = false;
    bool ok = true;
    bool output_json = false;
    bool selection_context_provided = false;
    std::string error;
    copperfin::studio::StudioSelectionToolboxObjectCreateBatchDispatchRequest request;
};
struct ToolboxCreateBatchDispatchFromDispatchPlanParseResult {
    bool requested = false;
    bool ok = true;
    bool output_json = false;
    bool selection_context_provided = false;
    bool admit_palette_invocation = false;
    std::string error;
    copperfin::studio::StudioToolboxPaletteLaunchRequest launch_request;
    copperfin::studio::StudioToolboxObjectCreateBatchDispatchFromPaletteDispatchRequest dispatch_request;
};
struct ToolboxCreateBatchParseResult {
    bool requested = false;
    bool ok = true;
    bool output_json = false;
    std::string error;
    copperfin::studio::StudioToolboxObjectCreateBatchPlanRequest request;
};
struct ToolboxCreatePlanCatalogParseResult {
    bool requested = false;
    bool ok = true;
    bool output_json = false;
    bool context_provided = false;
    std::string error;
    copperfin::studio::StudioToolboxObjectCreatePlanCatalogRequest request;
};
struct SelectionToolboxCreatePlanCatalogParseResult {
    bool requested = false;
    bool ok = true;
    bool output_json = false;
    bool selection_context_provided = false;
    std::string error;
    copperfin::studio::StudioSelectionToolboxObjectCreatePlanCatalogRequest request;
};
struct ToolboxCreateBatchPlanCatalogParseResult {
    bool requested = false;
    bool ok = true;
    bool output_json = false;
    bool context_provided = false;
    std::string error;
    copperfin::studio::StudioToolboxObjectCreateBatchPlanCatalogRequest request;
};
struct SelectionToolboxCreateBatchPlanCatalogParseResult {
    bool requested = false;
    bool ok = true;
    bool output_json = false;
    bool selection_context_provided = false;
    std::string error;
    copperfin::studio::StudioSelectionToolboxObjectCreateBatchPlanCatalogRequest request;
};
struct ToolboxCreateDispatchCatalogParseResult {
    bool requested = false;
    bool ok = true;
    bool output_json = false;
    bool context_provided = false;
    std::string error;
    copperfin::studio::StudioToolboxObjectCreateDispatchCatalogRequest request;
};
struct SelectionToolboxCreateDispatchCatalogParseResult {
    bool requested = false;
    bool ok = true;
    bool output_json = false;
    bool selection_context_provided = false;
    std::string error;
    copperfin::studio::StudioSelectionToolboxObjectCreateDispatchCatalogRequest request;
};
struct ToolboxCreateBatchDispatchCatalogParseResult {
    bool requested = false;
    bool ok = true;
    bool output_json = false;
    bool context_provided = false;
    std::string error;
    copperfin::studio::StudioToolboxObjectCreateBatchDispatchCatalogRequest request;
};
struct SelectionToolboxCreateBatchDispatchCatalogParseResult {
    bool requested = false;
    bool ok = true;
    bool output_json = false;
    bool selection_context_provided = false;
    std::string error;
    copperfin::studio::StudioSelectionToolboxObjectCreateBatchDispatchCatalogRequest request;
};
struct BuilderLaunchPlanParseResult {
    bool requested = false;
    bool ok = true;
    bool output_json = false;
    bool context_provided = false;
    bool selection_context_provided = false;
    std::string error;
    copperfin::studio::StudioBuilderLaunchRequest request;
    copperfin::studio::StudioEditorSelectionContext selection_context =
        copperfin::studio::StudioEditorSelectionContext::visual_object;
};
struct BuilderLaunchCatalogParseResult {
    bool requested = false;
    bool ok = true;
    bool output_json = false;
    bool context_provided = false;
    std::string error;
    copperfin::studio::StudioBuilderLaunchCatalogRequest request;
};
struct SelectionBuilderLaunchCatalogParseResult {
    bool requested = false;
    bool ok = true;
    bool output_json = false;
    bool selection_context_provided = false;
    std::string error;
    copperfin::studio::StudioSelectionBuilderLaunchCatalogRequest request;
};
struct BuilderInvocationAdmissionParseResult {
    bool requested = false;
    bool ok = true;
    bool output_json = false;
    bool context_provided = false;
    bool selection_context_provided = false;
    bool admit_ui_launch = false;
    std::string error;
    copperfin::studio::StudioBuilderLaunchRequest request;
    copperfin::studio::StudioEditorSelectionContext selection_context =
        copperfin::studio::StudioEditorSelectionContext::visual_object;
};
struct BuilderInvocationAdmissionCatalogParseResult {
    bool requested = false;
    bool ok = true;
    bool output_json = false;
    bool context_provided = false;
    std::string error;
    copperfin::studio::StudioBuilderInvocationAdmissionCatalogRequest request;
};
struct SelectionBuilderInvocationAdmissionCatalogParseResult {
    bool requested = false;
    bool ok = true;
    bool output_json = false;
    bool selection_context_provided = false;
    std::string error;
    copperfin::studio::StudioSelectionBuilderInvocationAdmissionCatalogRequest request;
};
struct BuilderDispatchParseResult {
    bool requested = false;
    bool ok = true;
    bool output_json = false;
    bool context_provided = false;
    bool selection_context_provided = false;
    bool admit_ui_launch = false;
    std::string error;
    copperfin::studio::StudioBuilderLaunchRequest request;
    copperfin::studio::StudioEditorSelectionContext selection_context =
        copperfin::studio::StudioEditorSelectionContext::visual_object;
};
struct BuilderExecuteParseResult {
    bool requested = false;
    bool ok = true;
    bool output_json = false;
    bool context_provided = false;
    bool selection_context_provided = false;
    bool admit_ui_launch = false;
    bool admit_execution = false;
    std::string launch_command;
    std::string error;
    copperfin::studio::StudioBuilderLaunchRequest request;
    copperfin::studio::StudioEditorSelectionContext selection_context =
        copperfin::studio::StudioEditorSelectionContext::visual_object;
};
struct BuilderDispatchCatalogParseResult {
    bool requested = false;
    bool ok = true;
    bool output_json = false;
    bool context_provided = false;
    std::string error;
    copperfin::studio::StudioBuilderDispatchCatalogRequest request;
};
struct BuilderDispatchExecutionCatalogParseResult {
    bool requested = false;
    bool ok = true;
    bool output_json = false;
    bool context_provided = false;
    std::string error;
    copperfin::studio::StudioBuilderDispatchExecutionCatalogRequest request;
};
struct SelectionBuilderDispatchCatalogParseResult {
    bool requested = false;
    bool ok = true;
    bool output_json = false;
    bool selection_context_provided = false;
    std::string error;
    copperfin::studio::StudioSelectionBuilderDispatchCatalogRequest request;
};
struct SelectionBuilderDispatchExecutionCatalogParseResult {
    bool requested = false;
    bool ok = true;
    bool output_json = false;
    bool selection_context_provided = false;
    std::string error;
    copperfin::studio::StudioSelectionBuilderDispatchExecutionCatalogRequest request;
};
struct EditorActionLaunchPlanParseResult {
    bool requested = false;
    bool ok = true;
    bool output_json = false;
    bool selection_context_provided = false;
    std::string error;
    copperfin::studio::StudioEditorActionLaunchRequest request;
};
struct EditorActionLaunchCatalogParseResult {
    bool requested = false;
    bool ok = true;
    bool output_json = false;
    bool selection_context_provided = false;
    std::string error;
    copperfin::studio::StudioEditorActionLaunchCatalogRequest request;
};
struct EditorActionInvocationAdmissionParseResult {
    bool requested = false;
    bool ok = true;
    bool output_json = false;
    bool selection_context_provided = false;
    bool admit_editor_invocation = false;
    std::string error;
    copperfin::studio::StudioEditorActionLaunchRequest request;
};
struct EditorActionInvocationAdmissionCatalogParseResult {
    bool requested = false;
    bool ok = true;
    bool output_json = false;
    bool selection_context_provided = false;
    std::string error;
    copperfin::studio::StudioEditorActionInvocationAdmissionCatalogRequest request;
};
struct EditorActionDispatchParseResult {
    bool requested = false;
    bool ok = true;
    bool output_json = false;
    bool selection_context_provided = false;
    bool admit_editor_invocation = false;
    std::string error;
    copperfin::studio::StudioEditorActionLaunchRequest request;
};
struct EditorActionExecuteParseResult {
    bool requested = false;
    bool ok = true;
    bool output_json = false;
    bool selection_context_provided = false;
    bool admit_editor_invocation = false;
    bool admit_execution = false;
    std::string launch_command;
    std::string error;
    copperfin::studio::StudioEditorActionLaunchRequest request;
};
struct EditorActionDispatchCatalogParseResult {
    bool requested = false;
    bool ok = true;
    bool output_json = false;
    bool selection_context_provided = false;
    std::string error;
    copperfin::studio::StudioEditorActionDispatchCatalogRequest request;
};
struct EditorActionDispatchExecutionCatalogParseResult {
    bool requested = false;
    bool ok = true;
    bool output_json = false;
    bool selection_context_provided = false;
    std::string error;
    copperfin::studio::StudioEditorActionDispatchExecutionCatalogRequest request;
};
struct ToolboxPaletteLaunchPlanParseResult {
    bool requested = false;
    bool ok = true;
    bool output_json = false;
    bool selection_context_provided = false;
    std::string error;
    copperfin::studio::StudioToolboxPaletteLaunchRequest request;
};
struct ToolboxPaletteLaunchCatalogParseResult {
    bool requested = false;
    bool ok = true;
    bool output_json = false;
    std::string error;
    copperfin::studio::StudioToolboxPaletteLaunchCatalogRequest request;
};
struct ToolboxPaletteQueryParseResult {
    bool requested = false;
    bool ok = true;
    bool output_json = false;
    bool context_provided = false;
    std::string error;
    copperfin::studio::StudioToolboxPaletteQueryRequest request;
};
struct VisualPropertyFilterParseResult {
    bool requested = false;
    bool ok = true;
    bool output_json = false;
    bool path_provided = false;
    std::string error;
    copperfin::vfp::VisualObjectPropertyListFilterRequest request;
};
struct VisualPropertyQueryParseResult {
    bool requested = false;
    bool ok = true;
    bool output_json = false;
    bool path_provided = false;
    bool property_name_provided = false;
    std::string error;
    copperfin::vfp::VisualObjectPropertyQueryRequest request;
};
struct VisualPropertyUpdateBatchParseResult {
    bool requested = false;
    bool ok = true;
    bool output_json = false;
    bool path_provided = false;
    std::string error;
    copperfin::vfp::VisualObjectMultiEditRequest request;
};
struct VisualPropertyClearParseResult {
    bool requested = false;
    bool ok = true;
    bool output_json = false;
    bool path_provided = false;
    bool property_name_provided = false;
    std::string error;
    copperfin::vfp::VisualObjectPropertyClearRequest request;
};
struct VisualPropertyClearBatchParseResult {
    bool requested = false;
    bool ok = true;
    bool output_json = false;
    bool path_provided = false;
    std::string error;
    copperfin::vfp::VisualObjectPropertyClearBatchRequest request;
};
struct VisualPropertyCopyParseResult {
    bool requested = false;
    bool ok = true;
    bool output_json = false;
    bool path_provided = false;
    bool property_name_provided = false;
    std::string error;
    copperfin::vfp::VisualObjectPropertyCopyRequest request;
};
struct VisualPropertyCopyBatchParseResult {
    bool requested = false;
    bool ok = true;
    bool output_json = false;
    bool path_provided = false;
    std::string error;
    copperfin::vfp::VisualObjectPropertyCopyBatchRequest request;
};
struct VisualPropertyMoveParseResult {
    bool requested = false;
    bool ok = true;
    bool output_json = false;
    bool path_provided = false;
    bool property_name_provided = false;
    std::string error;
    copperfin::vfp::VisualObjectPropertyMoveRequest request;
};
struct VisualPropertyMoveBatchParseResult {
    bool requested = false;
    bool ok = true;
    bool output_json = false;
    bool path_provided = false;
    std::string error;
    copperfin::vfp::VisualObjectPropertyMoveBatchRequest request;
};
struct VisualPropertyRenameParseResult {
    bool requested = false;
    bool ok = true;
    bool output_json = false;
    bool path_provided = false;
    bool property_name_provided = false;
    bool new_property_name_provided = false;
    std::string error;
    copperfin::vfp::VisualObjectPropertyRenameRequest request;
};
struct VisualPropertyRenameBatchParseResult {
    bool requested = false;
    bool ok = true;
    bool output_json = false;
    bool path_provided = false;
    std::string error;
    copperfin::vfp::VisualObjectPropertyRenameBatchRequest request;
};
struct VisualPropertyReorderParseResult {
    bool requested = false;
    bool ok = true;
    bool output_json = false;
    bool path_provided = false;
    bool property_name_provided = false;
    bool placement_provided = false;
    std::string error;
    copperfin::vfp::VisualObjectPropertyReorderRequest request;
};
struct VisualPropertyReorderBatchParseResult {
    bool requested = false;
    bool ok = true;
    bool output_json = false;
    bool path_provided = false;
    std::string error;
    copperfin::vfp::VisualObjectPropertyReorderBatchRequest request;
};
struct VisualPropertyListParseResult {
    bool requested = false;
    bool ok = true;
    bool output_json = false;
    bool path_provided = false;
    std::string error;
    copperfin::vfp::VisualObjectPropertyListRequest request;
};
struct VisualObjectListParseResult {
    bool requested = false;
    bool ok = true;
    bool output_json = false;
    bool path_provided = false;
    std::string path;
    std::string error;
};
struct VisualObjectChildrenParseResult {
    bool requested = false;
    bool ok = true;
    bool output_json = false;
    bool path_provided = false;
    std::string error;
    copperfin::vfp::VisualObjectChildrenListRequest request;
};
struct VisualObjectDescendantsParseResult {
    bool requested = false;
    bool ok = true;
    bool output_json = false;
    bool path_provided = false;
    std::string error;
    copperfin::vfp::VisualObjectDescendantsListRequest request;
};
struct VisualObjectAncestorsParseResult {
    bool requested = false;
    bool ok = true;
    bool output_json = false;
    bool path_provided = false;
    std::string error;
    copperfin::vfp::VisualObjectAncestorsListRequest request;
};
struct VisualObjectReparentBatchParseResult {
    bool requested = false;
    bool ok = true;
    bool output_json = false;
    bool path_provided = false;
    std::string error;
    copperfin::vfp::VisualObjectReparentBatchRequest request;
};
struct VisualObjectDuplicateBatchParseResult {
    bool requested = false;
    bool ok = true;
    bool output_json = false;
    bool path_provided = false;
    std::string error;
    copperfin::vfp::VisualObjectDuplicateBatchRequest request;
};
struct VisualObjectDuplicateSubtreeParseResult {
    bool requested = false;
    bool ok = true;
    bool output_json = false;
    bool path_provided = false;
    bool root_selector_provided = false;
    std::string error;
    copperfin::vfp::VisualObjectSubtreeDuplicateRequest request;
};
struct VisualObjectRenameBatchParseResult {
    bool requested = false;
    bool ok = true;
    bool output_json = false;
    bool path_provided = false;
    std::string error;
    copperfin::vfp::VisualObjectRenameBatchRequest request;
};
struct VisualObjectReorderBatchParseResult {
    bool requested = false;
    bool ok = true;
    bool output_json = false;
    bool path_provided = false;
    std::string error;
    copperfin::vfp::VisualObjectReorderBatchRequest request;
};
struct VisualObjectUpdateBatchParseResult {
    bool requested = false;
    bool ok = true;
    bool output_json = false;
    bool launched_from_visual_studio = false;
    bool path_provided = false;
    std::string error;
    copperfin::vfp::VisualObjectBatchEditRequest request;
};
struct VisualMethodListParseResult {
    bool requested = false;
    bool ok = true;
    bool output_json = false;
    bool path_provided = false;
    std::string error;
    copperfin::vfp::VisualObjectMethodListRequest request;
};
struct VisualMethodQueryParseResult {
    bool requested = false;
    bool ok = true;
    bool output_json = false;
    bool path_provided = false;
    bool method_name_provided = false;
    std::string error;
    copperfin::vfp::VisualObjectMethodQueryRequest request;
};
struct VisualMethodUpdateParseResult {
    bool requested = false;
    bool ok = true;
    bool output_json = false;
    bool path_provided = false;
    bool method_name_provided = false;
    bool method_kind_provided = false;
    bool method_source_provided = false;
    std::string error;
    copperfin::vfp::VisualObjectMethodEditRequest request;
};
struct VisualMethodDeleteParseResult {
    bool requested = false;
    bool ok = true;
    bool output_json = false;
    bool path_provided = false;
    bool method_name_provided = false;
    std::string error;
    copperfin::vfp::VisualObjectMethodDeleteRequest request;
};
struct VisualMethodDeleteBatchParseResult {
    bool requested = false;
    bool ok = true;
    bool output_json = false;
    bool path_provided = false;
    std::string error;
    copperfin::vfp::VisualObjectMethodDeleteBatchRequest request;
};
struct VisualMethodRenameParseResult {
    bool requested = false;
    bool ok = true;
    bool output_json = false;
    bool path_provided = false;
    bool method_name_provided = false;
    bool new_method_name_provided = false;
    std::string error;
    copperfin::vfp::VisualObjectMethodRenameRequest request;
};
struct VisualMethodRenameBatchParseResult {
    bool requested = false;
    bool ok = true;
    bool output_json = false;
    bool path_provided = false;
    std::string error;
    copperfin::vfp::VisualObjectMethodRenameBatchRequest request;
};
struct VisualMethodCopyParseResult {
    bool requested = false;
    bool ok = true;
    bool output_json = false;
    bool path_provided = false;
    bool method_name_provided = false;
    std::string error;
    copperfin::vfp::VisualObjectMethodCopyRequest request;
};
struct VisualMethodCopyBatchParseResult {
    bool requested = false;
    bool ok = true;
    bool output_json = false;
    bool path_provided = false;
    std::string error;
    copperfin::vfp::VisualObjectMethodCopyBatchRequest request;
};
struct VisualMethodMoveBatchParseResult {
    bool requested = false;
    bool ok = true;
    bool output_json = false;
    bool path_provided = false;
    std::string error;
    copperfin::vfp::VisualObjectMethodMoveBatchRequest request;
};
struct VisualMethodMoveParseResult {
    bool requested = false;
    bool ok = true;
    bool output_json = false;
    bool path_provided = false;
    bool method_name_provided = false;
    std::string error;
    copperfin::vfp::VisualObjectMethodMoveRequest request;
};
struct VisualMethodReorderParseResult {
    bool requested = false;
    bool ok = true;
    bool output_json = false;
    bool path_provided = false;
    bool method_name_provided = false;
    bool placement_provided = false;
    std::string error;
    copperfin::vfp::VisualObjectMethodReorderRequest request;
};
struct VisualMethodReorderBatchParseResult {
    bool requested = false;
    bool ok = true;
    bool output_json = false;
    bool path_provided = false;
    std::string error;
    copperfin::vfp::VisualObjectMethodReorderBatchRequest request;
};
struct ToolboxInvocationAdmissionParseResult {
    bool requested = false;
    bool ok = true;
    bool output_json = false;
    bool selection_context_provided = false;
    bool admit_palette_invocation = false;
    std::string error;
    copperfin::studio::StudioToolboxPaletteLaunchRequest request;
};
struct ToolboxInvocationAdmissionCatalogParseResult {
    bool requested = false;
    bool ok = true;
    bool output_json = false;
    bool context_provided = false;
    std::string error;
    copperfin::studio::StudioToolboxInvocationAdmissionCatalogRequest request;
};
struct SelectionToolboxInvocationAdmissionCatalogParseResult {
    bool requested = false;
    bool ok = true;
    bool output_json = false;
    bool selection_context_provided = false;
    std::string error;
    copperfin::studio::StudioSelectionToolboxInvocationAdmissionCatalogRequest request;
};
struct ToolboxDispatchParseResult {
    bool requested = false;
    bool ok = true;
    bool output_json = false;
    bool selection_context_provided = false;
    bool admit_palette_invocation = false;
    std::string error;
    copperfin::studio::StudioToolboxPaletteLaunchRequest request;
};
struct ToolboxExecuteParseResult {
    bool requested = false;
    bool ok = true;
    bool output_json = false;
    bool selection_context_provided = false;
    bool admit_palette_invocation = false;
    bool admit_execution = false;
    std::string launch_command;
    std::string error;
    copperfin::studio::StudioToolboxPaletteLaunchRequest request;
};
struct ToolboxDispatchCatalogParseResult {
    bool requested = false;
    bool ok = true;
    bool output_json = false;
    bool context_provided = false;
    std::string error;
    copperfin::studio::StudioToolboxDispatchCatalogRequest request;
};
struct ToolboxDispatchExecutionCatalogParseResult {
    bool requested = false;
    bool ok = true;
    bool output_json = false;
    bool context_provided = false;
    std::string error;
    copperfin::studio::StudioToolboxDispatchExecutionCatalogRequest request;
};
struct SelectionToolboxDispatchExecutionCatalogParseResult {
    bool requested = false;
    bool ok = true;
    bool output_json = false;
    bool selection_context_provided = false;
    std::string error;
    copperfin::studio::StudioSelectionToolboxDispatchExecutionCatalogRequest request;
};
struct SelectionToolboxDispatchCatalogParseResult {
    bool requested = false;
    bool ok = true;
    bool output_json = false;
    bool selection_context_provided = false;
    std::string error;
    copperfin::studio::StudioSelectionToolboxDispatchCatalogRequest request;
};
struct DesignerLaunchSurfacesParseResult {
    bool requested = false;
    bool ok = true;
    bool output_json = false;
    bool selection_context_provided = false;
    std::string error;
    copperfin::studio::StudioDesignerLaunchSurfaceRequest request;
};
struct DesignerInvocationAdmissionParseResult {
    bool requested = false;
    bool ok = true;
    bool output_json = false;
    bool selection_context_provided = false;
    bool admit_editor_invocations = false;
    bool admit_builder_invocations = false;
    bool admit_toolbox_invocation = false;
    std::string error;
    copperfin::studio::StudioDesignerLaunchSurfaceRequest request;
};
struct DesignerDispatchParseResult {
    bool requested = false;
    bool ok = true;
    bool output_json = false;
    bool selection_context_provided = false;
    bool admit_editor_invocations = false;
    bool admit_builder_invocations = false;
    bool admit_toolbox_invocation = false;
    std::string error;
    copperfin::studio::StudioDesignerLaunchSurfaceRequest request;
};
struct DesignerExecuteParseResult {
    bool requested = false;
    bool ok = true;
    bool output_json = false;
    bool selection_context_provided = false;
    bool admit_editor_invocations = false;
    bool admit_builder_invocations = false;
    bool admit_toolbox_invocation = false;
    bool admit_execution = false;
    std::string editor_action_launch_command;
    std::string builder_launch_command;
    std::string toolbox_launch_command;
    std::string error;
    copperfin::studio::StudioDesignerLaunchSurfaceRequest request;
};
struct DesignerLaunchSurfaceCatalogParseResult {
    bool requested = false;
    bool ok = true;
    bool output_json = false;
    std::string error;
    copperfin::studio::StudioDesignerLaunchSurfaceCatalogRequest request;
};
struct DesignerInvocationAdmissionCatalogParseResult {
    bool requested = false;
    bool ok = true;
    bool output_json = false;
    bool admit_editor_invocations = false;
    bool admit_builder_invocations = false;
    bool admit_toolbox_invocation = false;
    std::string error;
    copperfin::studio::StudioDesignerInvocationAdmissionCatalogRequest request;
};
struct DesignerDispatchCatalogParseResult {
    bool requested = false;
    bool ok = true;
    bool output_json = false;
    bool admit_editor_invocations = false;
    bool admit_builder_invocations = false;
    bool admit_toolbox_invocation = false;
    std::string error;
    copperfin::studio::StudioDesignerDispatchCatalogRequest request;
};
struct DesignerDispatchExecutionCatalogParseResult {
    bool requested = false;
    bool ok = true;
    bool output_json = false;
    bool admit_editor_invocations = false;
    bool admit_builder_invocations = false;
    bool admit_toolbox_invocation = false;
    bool admit_designer_execution = false;
    std::string error;
    copperfin::studio::StudioDesignerDispatchExecutionCatalogRequest request;
};

// ==== Builder subsystem: argument parsing, dispatch, and JSON/text output ====
std::string builder_parse_missing_value(
    const copperfin::localization::LocalizedCatalog& catalog,
    const std::string& option);
std::string builder_parse_unknown_builder_context_token(
    const copperfin::localization::LocalizedCatalog& catalog,
    const std::string& token);
std::string builder_parse_unknown_selection_context_token(
    const copperfin::localization::LocalizedCatalog& catalog,
    const std::string& token);
std::string builder_parse_record_non_negative_integer(
    const copperfin::localization::LocalizedCatalog& catalog);
std::string builder_parse_boolean_value_required(
    const copperfin::localization::LocalizedCatalog& catalog,
    const std::string& option);
std::string builder_parse_unknown_option(
    const copperfin::localization::LocalizedCatalog& catalog,
    const std::string& command_name,
    const std::string& argument);
std::string builder_parse_context_conflict(
    const copperfin::localization::LocalizedCatalog& catalog,
    std::string_view request_name_key);
std::string builder_parse_message(
    const copperfin::localization::LocalizedCatalog& catalog,
    std::string_view key);
SelectionBuilderLaunchCatalogParseResult
parse_selection_builder_launch_catalog_arguments(
    const copperfin::localization::LocalizedCatalog& catalog,
    const std::vector<std::string>& args);
SelectionBuilderInvocationAdmissionCatalogParseResult
parse_selection_builder_invocation_admission_catalog_arguments(
    const copperfin::localization::LocalizedCatalog& catalog,
    const std::vector<std::string>& args);
SelectionBuilderDispatchExecutionCatalogParseResult
parse_selection_builder_dispatch_execution_catalog_arguments(
    const copperfin::localization::LocalizedCatalog& catalog,
    const std::vector<std::string>& args);
SelectionBuilderDispatchCatalogParseResult
parse_selection_builder_dispatch_catalog_arguments(
    const copperfin::localization::LocalizedCatalog& catalog,
    const std::vector<std::string>& args);
void print_json_builder_launch_plan_result(
    const copperfin::studio::StudioBuilderLaunchPlanResult& result,
    const copperfin::studio::StudioEditorSelectionContext* selection_context = nullptr);
void print_json_builder_launch_catalog_entry(
    const copperfin::studio::StudioBuilderLaunchCatalogEntry& entry,
    const std::string& indent);
void print_json_builder_launch_catalog_result(
    const copperfin::studio::StudioBuilderLaunchCatalogResult& result);
void print_json_builder_invocation_admission_result(
    const copperfin::studio::StudioBuilderInvocationAdmissionResult& result,
    const copperfin::studio::StudioEditorSelectionContext* selection_context = nullptr);
void print_json_builder_invocation_admission_catalog_entry(
    const copperfin::studio::StudioBuilderInvocationAdmissionCatalogEntry& entry,
    const std::string& indent);
void print_json_builder_invocation_admission_catalog_result(
    const copperfin::studio::StudioBuilderInvocationAdmissionCatalogResult& result);
void print_json_builder_dispatch_result(
    const copperfin::studio::StudioBuilderDispatchResult& result,
    const copperfin::studio::StudioEditorSelectionContext* selection_context = nullptr);
void print_json_builder_execution_result(
    const copperfin::studio::StudioBuilderDispatchExecutionResult& result,
    const std::string& launch_command,
    const std::string& executed_command,
    const copperfin::studio::StudioEditorSelectionContext* selection_context = nullptr);
void print_json_builder_dispatch_catalog_entry(
    const copperfin::studio::StudioBuilderDispatchCatalogEntry& entry,
    const std::string& indent);
void print_json_builder_dispatch_catalog_result(
    const copperfin::studio::StudioBuilderDispatchCatalogResult& result);
void print_json_builder_dispatch_execution_catalog_entry(
    const copperfin::studio::StudioBuilderDispatchExecutionCatalogEntry& entry,
    const std::string& indent);
void print_json_builder_dispatch_execution_catalog_result(
    const copperfin::studio::StudioBuilderDispatchExecutionCatalogResult& result);
void print_text_builder_launch_plan_result(
    const copperfin::studio::StudioBuilderLaunchPlanResult& result,
    const copperfin::studio::StudioEditorSelectionContext* selection_context = nullptr);
void print_text_builder_launch_catalog_result(
    const copperfin::studio::StudioBuilderLaunchCatalogResult& result);
void print_text_builder_invocation_admission_result(
    const copperfin::studio::StudioBuilderInvocationAdmissionResult& result,
    const copperfin::studio::StudioEditorSelectionContext* selection_context = nullptr);
void print_text_builder_invocation_admission_catalog_result(
    const copperfin::studio::StudioBuilderInvocationAdmissionCatalogResult& result);
void print_text_builder_dispatch_result(
    const copperfin::studio::StudioBuilderDispatchResult& result,
    const copperfin::studio::StudioEditorSelectionContext* selection_context = nullptr);
void print_text_builder_execution_result(
    const copperfin::studio::StudioBuilderDispatchExecutionResult& result,
    const std::string& launch_command,
    const std::string& executed_command,
    const copperfin::studio::StudioEditorSelectionContext* selection_context = nullptr);
void print_text_builder_dispatch_catalog_result(
    const copperfin::studio::StudioBuilderDispatchCatalogResult& result);
void print_text_builder_dispatch_execution_catalog_result(
    const copperfin::studio::StudioBuilderDispatchExecutionCatalogResult& result);
std::optional<int> try_handle_builder_launch(
    const copperfin::localization::LocalizedCatalog& catalog,
    const std::vector<std::string>& args);
std::optional<int> try_handle_builder_launch_catalog(
    const copperfin::localization::LocalizedCatalog& catalog,
    const std::vector<std::string>& args);
std::optional<int> try_handle_selection_builder_launch_catalog(
    const copperfin::localization::LocalizedCatalog& catalog,
    const std::vector<std::string>& args);
std::optional<int> try_handle_builder_invocation_admission(
    const copperfin::localization::LocalizedCatalog& catalog,
    const std::vector<std::string>& args);
std::optional<int> try_handle_builder_invocation_admission_catalog(
    const copperfin::localization::LocalizedCatalog& catalog,
    const std::vector<std::string>& args);
std::optional<int> try_handle_selection_builder_invocation_admission_catalog(
    const copperfin::localization::LocalizedCatalog& catalog,
    const std::vector<std::string>& args);
std::optional<int> try_handle_builder_dispatch(
    const copperfin::localization::LocalizedCatalog& catalog,
    const std::vector<std::string>& args);
std::optional<int> try_handle_builder_execute(
    const copperfin::localization::LocalizedCatalog& catalog,
    const std::vector<std::string>& args);
std::optional<int> try_handle_builder_dispatch_catalog(
    const copperfin::localization::LocalizedCatalog& catalog,
    const std::vector<std::string>& args);
std::optional<int> try_handle_builder_dispatch_execution_catalog(
    const copperfin::localization::LocalizedCatalog& catalog,
    const std::vector<std::string>& args);
std::optional<int> try_handle_selection_builder_dispatch_execution_catalog(
    const copperfin::localization::LocalizedCatalog& catalog,
    const std::vector<std::string>& args);
std::optional<int> try_handle_selection_builder_dispatch_catalog(
    const copperfin::localization::LocalizedCatalog& catalog,
    const std::vector<std::string>& args);

// ==== Designer subsystem: argument parsing, dispatch, and JSON/text output ====
std::string designer_parse_missing_value(
    const copperfin::localization::LocalizedCatalog& catalog,
    const std::string& option);
std::string designer_parse_unknown_selection_context_token(
    const copperfin::localization::LocalizedCatalog& catalog,
    const std::string& token);
std::string designer_parse_non_negative_integer(
    const copperfin::localization::LocalizedCatalog& catalog,
    const std::string& option);
std::string designer_parse_boolean_value_required(
    const copperfin::localization::LocalizedCatalog& catalog,
    const std::string& option);
std::string designer_parse_unknown_option(
    const copperfin::localization::LocalizedCatalog& catalog,
    const std::string& command_name,
    const std::string& argument);
std::string designer_parse_message(
    const copperfin::localization::LocalizedCatalog& catalog,
    std::string_view key);
void print_json_designer_launch_surface_action(
    const copperfin::studio::StudioEditorActionLaunchPlanResult& result,
    const std::string& indent);
void print_json_designer_launch_surface_builder(
    const copperfin::studio::StudioSelectionBuilderLaunchPlanResult& result,
    const std::string& indent);
void print_json_designer_launch_surfaces_result(
    const copperfin::studio::StudioDesignerLaunchSurfacePlanResult& result);
void print_json_designer_invocation_admission_action(
    const copperfin::studio::StudioEditorActionInvocationAdmissionResult& result,
    const std::string& indent);
void print_json_designer_invocation_admission_builder(
    const copperfin::studio::StudioBuilderInvocationAdmissionResult& result,
    const std::string& indent);
void print_json_designer_invocation_admission_result(
    const copperfin::studio::StudioDesignerInvocationAdmissionResult& result);
void print_json_designer_dispatch_action(
    const copperfin::studio::StudioEditorActionDispatchResult& result,
    const std::string& indent);
void print_json_designer_dispatch_builder(
    const copperfin::studio::StudioBuilderDispatchResult& result,
    const std::string& indent);
void print_json_designer_dispatch_result(
    const copperfin::studio::StudioDesignerDispatchResult& result);
void print_json_designer_execution_result(
    const copperfin::studio::StudioDesignerDispatchExecutionResult& result,
    const copperfin::studio::StudioDesignerDispatchPlan* planned_dispatch_plan,
    const copperfin::localization::LocalizedCatalog& catalog,
    const std::string& editor_action_launch_command,
    const std::string& builder_launch_command,
    const std::string& toolbox_launch_command);
void print_json_designer_dispatch_catalog_context(
    const copperfin::studio::StudioDesignerDispatchCatalogEntry& entry,
    const std::string& indent);
void print_json_designer_execution_catalog_editor_dispatch(
    const copperfin::studio::StudioEditorActionDispatchResult& dispatch,
    const std::string& indent);
void print_json_designer_execution_catalog_builder_dispatch(
    const copperfin::studio::StudioBuilderDispatchResult& dispatch,
    const std::string& indent);
void print_json_designer_dispatch_catalog_result(
    const copperfin::studio::StudioDesignerDispatchCatalogResult& result);
void print_json_designer_dispatch_execution_catalog_context(
    const copperfin::studio::StudioDesignerDispatchExecutionCatalogEntry& entry,
    const std::string& indent);
void print_json_designer_dispatch_execution_catalog_result(
    const copperfin::studio::StudioDesignerDispatchExecutionCatalogResult& result);
void print_json_designer_invocation_admission_catalog_context(
    const copperfin::studio::StudioDesignerInvocationAdmissionCatalogEntry& entry,
    const std::string& indent);
void print_json_designer_invocation_admission_catalog_result(
    const copperfin::studio::StudioDesignerInvocationAdmissionCatalogResult& result);
void print_json_designer_launch_surface_catalog_context(
    const copperfin::studio::StudioDesignerLaunchSurfaceCatalogEntry& entry,
    const std::string& indent);
void print_json_designer_launch_surface_catalog_result(
    const copperfin::studio::StudioDesignerLaunchSurfaceCatalogResult& result);
void print_text_designer_launch_surfaces_result(
    const copperfin::studio::StudioDesignerLaunchSurfacePlanResult& result);
void print_text_designer_invocation_admission_result(
    const copperfin::studio::StudioDesignerInvocationAdmissionResult& result);
void print_text_designer_dispatch_result(
    const copperfin::studio::StudioDesignerDispatchResult& result);
void print_text_designer_execution_result(
    const copperfin::studio::StudioDesignerDispatchExecutionResult& result);
void print_text_designer_dispatch_catalog_result(
    const copperfin::studio::StudioDesignerDispatchCatalogResult& result);
void print_text_designer_dispatch_execution_catalog_result(
    const copperfin::studio::StudioDesignerDispatchExecutionCatalogResult& result);
void print_text_designer_launch_surface_catalog_result(
    const copperfin::studio::StudioDesignerLaunchSurfaceCatalogResult& result);
void print_text_designer_invocation_admission_catalog_result(
    const copperfin::studio::StudioDesignerInvocationAdmissionCatalogResult& result);
void print_json_designer_contexts(const std::vector<copperfin::studio::StudioDesignerContextResult>& contexts);
std::optional<int> try_handle_designer_launch_surfaces(
    const copperfin::localization::LocalizedCatalog& catalog,
    const std::vector<std::string>& args);
std::optional<int> try_handle_designer_invocation_admission(
    const copperfin::localization::LocalizedCatalog& catalog,
    const std::vector<std::string>& args);
std::optional<int> try_handle_designer_dispatch(
    const copperfin::localization::LocalizedCatalog& catalog,
    const std::vector<std::string>& args);
std::optional<int> try_handle_designer_execute(
    const copperfin::localization::LocalizedCatalog& catalog,
    const std::vector<std::string>& args);
std::optional<int> try_handle_designer_dispatch_execution_catalog(
    const copperfin::localization::LocalizedCatalog& catalog,
    const std::vector<std::string>& args);
std::optional<int> try_handle_designer_dispatch_catalog(
    const copperfin::localization::LocalizedCatalog& catalog,
    const std::vector<std::string>& args);
std::optional<int> try_handle_designer_invocation_admission_catalog(
    const copperfin::localization::LocalizedCatalog& catalog,
    const std::vector<std::string>& args);
std::optional<int> try_handle_designer_launch_surface_catalog(
    const copperfin::localization::LocalizedCatalog& catalog,
    const std::vector<std::string>& args);

// ==== Editor-action subsystem: argument parsing, dispatch, and JSON/text output ====
std::string editor_action_parse_missing_value(
    const copperfin::localization::LocalizedCatalog& catalog,
    const std::string& option);
std::string editor_action_parse_unknown_selection_context_token(
    const copperfin::localization::LocalizedCatalog& catalog,
    const std::string& token);
std::string editor_action_parse_non_negative_integer(
    const copperfin::localization::LocalizedCatalog& catalog,
    const std::string& option);
std::string editor_action_parse_boolean_value_required(
    const copperfin::localization::LocalizedCatalog& catalog,
    const std::string& option);
std::string editor_action_parse_unknown_option(
    const copperfin::localization::LocalizedCatalog& catalog,
    const std::string& command_name,
    const std::string& argument);
std::string editor_action_parse_message(
    const copperfin::localization::LocalizedCatalog& catalog,
    std::string_view key);
void print_json_editor_action_launch_plan_result(
    const copperfin::studio::StudioEditorActionLaunchPlanResult& result);
void print_json_editor_action_launch_catalog_entry(
    const copperfin::studio::StudioEditorActionLaunchCatalogEntry& entry,
    const std::string& indent);
void print_json_editor_action_launch_catalog_result(
    const copperfin::studio::StudioEditorActionLaunchCatalogResult& result);
void print_json_editor_action_invocation_admission_result(
    const copperfin::studio::StudioEditorActionInvocationAdmissionResult& result);
void print_json_editor_action_invocation_admission_catalog_entry(
    const copperfin::studio::StudioEditorActionInvocationAdmissionCatalogEntry& entry,
    const std::string& indent);
void print_json_editor_action_invocation_admission_catalog_result(
    const copperfin::studio::StudioEditorActionInvocationAdmissionCatalogResult& result);
void print_json_editor_action_dispatch_result(
    const copperfin::studio::StudioEditorActionDispatchResult& result);
void print_json_editor_action_execution_result(
    const copperfin::studio::StudioEditorActionDispatchExecutionResult& result,
    const std::string& launch_command,
    const std::string& executed_command);
void print_json_editor_action_dispatch_catalog_entry(
    const copperfin::studio::StudioEditorActionDispatchCatalogEntry& entry,
    const std::string& indent);
void print_json_editor_action_dispatch_catalog_result(
    const copperfin::studio::StudioEditorActionDispatchCatalogResult& result);
void print_json_editor_action_dispatch_execution_catalog_entry(
    const copperfin::studio::StudioEditorActionDispatchExecutionCatalogEntry& entry,
    const std::string& indent);
void print_json_editor_action_dispatch_execution_catalog_result(
    const copperfin::studio::StudioEditorActionDispatchExecutionCatalogResult& result);
void print_text_editor_action_launch_plan_result(
    const copperfin::studio::StudioEditorActionLaunchPlanResult& result);
void print_text_editor_action_launch_catalog_result(
    const copperfin::studio::StudioEditorActionLaunchCatalogResult& result);
void print_text_editor_action_invocation_admission_result(
    const copperfin::studio::StudioEditorActionInvocationAdmissionResult& result);
void print_text_editor_action_invocation_admission_catalog_result(
    const copperfin::studio::StudioEditorActionInvocationAdmissionCatalogResult& result);
void print_text_editor_action_dispatch_result(
    const copperfin::studio::StudioEditorActionDispatchResult& result);
void print_text_editor_action_execution_result(
    const copperfin::studio::StudioEditorActionDispatchExecutionResult& result,
    const std::string& launch_command,
    const std::string& executed_command);
void print_text_editor_action_dispatch_catalog_result(
    const copperfin::studio::StudioEditorActionDispatchCatalogResult& result);
void print_text_editor_action_dispatch_execution_catalog_result(
    const copperfin::studio::StudioEditorActionDispatchExecutionCatalogResult& result);
void print_json_editor_contexts(const std::vector<copperfin::studio::StudioEditorSelectionContext>& contexts);
std::optional<int> try_handle_editor_action_launch(
    const copperfin::localization::LocalizedCatalog& catalog,
    const std::vector<std::string>& args);
std::optional<int> try_handle_editor_action_launch_catalog(
    const copperfin::localization::LocalizedCatalog& catalog,
    const std::vector<std::string>& args);
std::optional<int> try_handle_editor_action_invocation_admission(
    const copperfin::localization::LocalizedCatalog& catalog,
    const std::vector<std::string>& args);
std::optional<int> try_handle_editor_action_invocation_admission_catalog(
    const copperfin::localization::LocalizedCatalog& catalog,
    const std::vector<std::string>& args);
std::optional<int> try_handle_editor_action_dispatch(
    const copperfin::localization::LocalizedCatalog& catalog,
    const std::vector<std::string>& args);
std::optional<int> try_handle_editor_action_execute(
    const copperfin::localization::LocalizedCatalog& catalog,
    const std::vector<std::string>& args);
std::optional<int> try_handle_editor_action_dispatch_catalog(
    const copperfin::localization::LocalizedCatalog& catalog,
    const std::vector<std::string>& args);
std::optional<int> try_handle_editor_action_dispatch_execution_catalog(
    const copperfin::localization::LocalizedCatalog& catalog,
    const std::vector<std::string>& args);

// ==== Toolbox subsystem: shared parsing/printing helpers ====
std::string toolbox_parse_missing_value(
    const copperfin::localization::LocalizedCatalog& catalog,
    const std::string& option);
std::string toolbox_parse_unknown_selection_context_token(
    const copperfin::localization::LocalizedCatalog& catalog,
    const std::string& token);
std::string toolbox_parse_unknown_toolbox_context_token(
    const copperfin::localization::LocalizedCatalog& catalog,
    const std::string& token);
std::string toolbox_parse_non_negative_integer(
    const copperfin::localization::LocalizedCatalog& catalog,
    const std::string& option);
std::string toolbox_parse_boolean_value_required(
    const copperfin::localization::LocalizedCatalog& catalog,
    const std::string& option);
std::string toolbox_parse_unknown_option(
    const copperfin::localization::LocalizedCatalog& catalog,
    const std::string& command_name,
    const std::string& argument);
std::string toolbox_parse_batch_item_requires_toolbox_item(
    const copperfin::localization::LocalizedCatalog& catalog);
std::string toolbox_parse_selection_batch_item_requires_toolbox_item(
    const copperfin::localization::LocalizedCatalog& catalog);
std::string toolbox_parse_message(
    const copperfin::localization::LocalizedCatalog& catalog,
    std::string_view key);
bool parse_toolbox_context_token(
    const std::string& token,
    copperfin::studio::StudioToolboxContext& context);
void print_json_toolbox_contexts(const std::vector<copperfin::studio::StudioToolboxContext>& contexts);
void print_json_toolbox_item_descriptor(
    const copperfin::studio::StudioToolboxItemDescriptor& item,
    const std::string& indent);

// ==== Toolbox subsystem: single-item create/plan/dispatch-catalog operations ====
std::string toolbox_parse_selection_batch_create_item_requires_toolbox_item(
    const copperfin::localization::LocalizedCatalog& catalog);
ToolboxCreatePlanParseResult parse_toolbox_create_plan_arguments(
    const copperfin::localization::LocalizedCatalog& catalog,
    const std::vector<std::string>& args);
ToolboxCreateFromDispatchPlanParseResult parse_toolbox_create_from_dispatch_plan_arguments(
    const copperfin::localization::LocalizedCatalog& catalog,
    const std::vector<std::string>& args);
ToolboxCreateFromDispatchParseResult parse_toolbox_create_from_dispatch_arguments(
    const copperfin::localization::LocalizedCatalog& catalog,
    const std::vector<std::string>& args);
ToolboxCreateDispatchFromDispatchPlanParseResult parse_toolbox_create_dispatch_from_dispatch_plan_arguments(
    const copperfin::localization::LocalizedCatalog& catalog,
    const std::vector<std::string>& args);
SelectionToolboxCreatePlanParseResult parse_selection_toolbox_create_plan_arguments(
    const copperfin::localization::LocalizedCatalog& catalog,
    const std::vector<std::string>& args);
SelectionToolboxCreateDispatchPlanParseResult parse_selection_toolbox_create_dispatch_plan_arguments(
    const copperfin::localization::LocalizedCatalog& catalog,
    const std::vector<std::string>& args);
ToolboxCreateDispatchPlanParseResult parse_toolbox_create_dispatch_plan_arguments(
    const copperfin::localization::LocalizedCatalog& catalog,
    const std::vector<std::string>& args);
ToolboxCreatePlanCatalogParseResult parse_toolbox_create_plan_catalog_arguments(
    const copperfin::localization::LocalizedCatalog& catalog,
    const std::vector<std::string>& args);
SelectionToolboxCreatePlanCatalogParseResult parse_selection_toolbox_create_plan_catalog_arguments(
    const copperfin::localization::LocalizedCatalog& catalog,
    const std::vector<std::string>& args);
ToolboxCreateDispatchCatalogParseResult parse_toolbox_create_dispatch_catalog_arguments(
    const copperfin::localization::LocalizedCatalog& catalog,
    const std::vector<std::string>& args);
SelectionToolboxCreateDispatchCatalogParseResult parse_selection_toolbox_create_dispatch_catalog_arguments(
    const copperfin::localization::LocalizedCatalog& catalog,
    const std::vector<std::string>& args);
SelectionToolboxCreateParseResult parse_selection_toolbox_create_arguments(
    const copperfin::localization::LocalizedCatalog& catalog,
    const std::vector<std::string>& args);
ToolboxCreateParseResult parse_toolbox_create_arguments(
    const copperfin::localization::LocalizedCatalog& catalog,
    const std::vector<std::string>& args);
void print_json_toolbox_create_result(const copperfin::vfp::VisualObjectCreateResult& result);
void print_json_toolbox_create_from_dispatch_result(
    const copperfin::studio::StudioToolboxObjectCreateFromDispatchResult& result);
void print_json_toolbox_create_plan_result(
    const copperfin::studio::StudioToolboxObjectCreatePlanResult& result);
void print_json_toolbox_create_dispatch_plan_result(
    const copperfin::studio::StudioToolboxObjectCreateDispatchResult& result);
void print_json_toolbox_create_plan_catalog_entry(
    const copperfin::studio::StudioToolboxObjectCreatePlanCatalogEntry& entry,
    const std::string& indent);
void print_json_toolbox_create_plan_catalog_result(
    const copperfin::studio::StudioToolboxObjectCreatePlanCatalogResult& result);
void print_json_toolbox_create_dispatch_catalog_entry(
    const copperfin::studio::StudioToolboxObjectCreateDispatchCatalogEntry& entry,
    const std::string& indent);
void print_json_toolbox_create_dispatch_catalog_result(
    const copperfin::studio::StudioToolboxObjectCreateDispatchCatalogResult& result);
void print_text_toolbox_create_result(const copperfin::vfp::VisualObjectCreateResult& result);
void print_text_toolbox_create_plan_result(
    const copperfin::studio::StudioToolboxObjectCreatePlanResult& result);
void print_text_toolbox_create_from_dispatch_result(
    const copperfin::studio::StudioToolboxObjectCreateFromDispatchResult& result);
void print_text_toolbox_create_dispatch_plan_result(
    const copperfin::studio::StudioToolboxObjectCreateDispatchResult& result);
void print_text_toolbox_create_plan_catalog_result(
    const copperfin::studio::StudioToolboxObjectCreatePlanCatalogResult& result);
void print_text_toolbox_create_dispatch_catalog_result(
    const copperfin::studio::StudioToolboxObjectCreateDispatchCatalogResult& result);
std::optional<int> try_handle_toolbox_create_dispatch_catalog(
    const copperfin::localization::LocalizedCatalog& catalog,
    const std::vector<std::string>& args);
std::optional<int> try_handle_selection_toolbox_create_dispatch_catalog(
    const copperfin::localization::LocalizedCatalog& catalog,
    const std::vector<std::string>& args);
std::optional<int> try_handle_toolbox_create_plan_catalog(
    const copperfin::localization::LocalizedCatalog& catalog,
    const std::vector<std::string>& args);
std::optional<int> try_handle_selection_toolbox_create_plan_catalog(
    const copperfin::localization::LocalizedCatalog& catalog,
    const std::vector<std::string>& args);
std::optional<int> try_handle_toolbox_create_from_dispatch(
    const copperfin::localization::LocalizedCatalog& catalog,
    const std::vector<std::string>& args);
std::optional<int> try_handle_toolbox_create_from_dispatch_plan(
    const copperfin::localization::LocalizedCatalog& catalog,
    const std::vector<std::string>& args);
std::optional<int> try_handle_toolbox_create_dispatch_from_dispatch_plan(
    const copperfin::localization::LocalizedCatalog& catalog,
    const std::vector<std::string>& args);
std::optional<int> try_handle_toolbox_create_dispatch_plan(
    const copperfin::localization::LocalizedCatalog& catalog,
    const std::vector<std::string>& args);
std::optional<int> try_handle_selection_toolbox_create_dispatch_plan(
    const copperfin::localization::LocalizedCatalog& catalog,
    const std::vector<std::string>& args);
std::optional<int> try_handle_selection_toolbox_create_plan(
    const copperfin::localization::LocalizedCatalog& catalog,
    const std::vector<std::string>& args);
std::optional<int> try_handle_toolbox_create_plan(
    const copperfin::localization::LocalizedCatalog& catalog,
    const std::vector<std::string>& args);
std::optional<int> try_handle_selection_toolbox_create(
    const copperfin::localization::LocalizedCatalog& catalog,
    const std::vector<std::string>& args);
std::optional<int> try_handle_toolbox_create(
    const copperfin::localization::LocalizedCatalog& catalog,
    const std::vector<std::string>& args);

// ==== Toolbox subsystem: batch create/plan/dispatch-catalog operations ====
ToolboxCreateBatchFromDispatchPlanParseResult parse_toolbox_create_batch_from_dispatch_plan_arguments(
    const copperfin::localization::LocalizedCatalog& catalog,
    const std::vector<std::string>& args);
ToolboxCreateBatchFromDispatchParseResult parse_toolbox_create_batch_from_dispatch_arguments(
    const copperfin::localization::LocalizedCatalog& catalog,
    const std::vector<std::string>& args);
ToolboxCreateBatchDispatchFromDispatchPlanParseResult
parse_toolbox_create_batch_dispatch_from_dispatch_plan_arguments(
    const copperfin::localization::LocalizedCatalog& catalog,
    const std::vector<std::string>& args);
ToolboxCreateBatchPlanParseResult parse_toolbox_create_batch_plan_arguments(
    const copperfin::localization::LocalizedCatalog& catalog,
    const std::vector<std::string>& args);
SelectionToolboxCreateBatchPlanParseResult parse_selection_toolbox_create_batch_plan_arguments(
    const copperfin::localization::LocalizedCatalog& catalog,
    const std::vector<std::string>& args);
SelectionToolboxCreateBatchParseResult parse_selection_toolbox_create_batch_arguments(
    const copperfin::localization::LocalizedCatalog& catalog,
    const std::vector<std::string>& args);
SelectionToolboxCreateBatchDispatchPlanParseResult parse_selection_toolbox_create_batch_dispatch_plan_arguments(
    const copperfin::localization::LocalizedCatalog& catalog,
    const std::vector<std::string>& args);
ToolboxCreateBatchDispatchPlanParseResult parse_toolbox_create_batch_dispatch_plan_arguments(
    const copperfin::localization::LocalizedCatalog& catalog,
    const std::vector<std::string>& args);
ToolboxCreateBatchParseResult parse_toolbox_create_batch_arguments(
    const copperfin::localization::LocalizedCatalog& catalog,
    const std::vector<std::string>& args);
ToolboxCreateBatchPlanCatalogParseResult parse_toolbox_create_batch_plan_catalog_arguments(
    const copperfin::localization::LocalizedCatalog& catalog,
    const std::vector<std::string>& args);
SelectionToolboxCreateBatchPlanCatalogParseResult parse_selection_toolbox_create_batch_plan_catalog_arguments(
    const copperfin::localization::LocalizedCatalog& catalog,
    const std::vector<std::string>& args);
ToolboxCreateBatchDispatchCatalogParseResult parse_toolbox_create_batch_dispatch_catalog_arguments(
    const copperfin::localization::LocalizedCatalog& catalog,
    const std::vector<std::string>& args);
SelectionToolboxCreateBatchDispatchCatalogParseResult
parse_selection_toolbox_create_batch_dispatch_catalog_arguments(
    const copperfin::localization::LocalizedCatalog& catalog,
    const std::vector<std::string>& args);
void print_json_toolbox_create_batch_result(const copperfin::vfp::VisualObjectCreateBatchResult& result);
void print_json_toolbox_create_batch_plan_entry(
    const copperfin::studio::StudioToolboxObjectCreatePlan& plan,
    const std::string& indent);
void print_json_toolbox_create_batch_plan_result(
    const copperfin::studio::StudioToolboxObjectCreateBatchPlanResult& result);
void print_json_toolbox_create_batch_from_dispatch_result(
    const copperfin::studio::StudioToolboxObjectCreateBatchFromDispatchResult& result);
void print_json_toolbox_create_batch_dispatch_plan_result(
    const copperfin::studio::StudioToolboxObjectCreateBatchDispatchResult& result);
void print_json_toolbox_create_batch_dispatch_catalog_result(
    const copperfin::studio::StudioToolboxObjectCreateBatchDispatchCatalogResult& result);
void print_json_toolbox_create_batch_plan_catalog_result(
    const copperfin::studio::StudioToolboxObjectCreateBatchPlanCatalogResult& result);
void print_text_toolbox_create_batch_result(const copperfin::vfp::VisualObjectCreateBatchResult& result);
void print_text_toolbox_create_batch_plan_result(
    const copperfin::studio::StudioToolboxObjectCreateBatchPlanResult& result);
void print_text_toolbox_create_batch_from_dispatch_result(
    const copperfin::studio::StudioToolboxObjectCreateBatchFromDispatchResult& result);
void print_text_toolbox_create_batch_dispatch_plan_result(
    const copperfin::studio::StudioToolboxObjectCreateBatchDispatchResult& result);
void print_text_toolbox_create_batch_plan_catalog_result(
    const copperfin::studio::StudioToolboxObjectCreateBatchPlanCatalogResult& result);
void print_text_toolbox_create_batch_dispatch_catalog_result(
    const copperfin::studio::StudioToolboxObjectCreateBatchDispatchCatalogResult& result);
std::optional<int> try_handle_toolbox_create_batch_plan_catalog(
    const copperfin::localization::LocalizedCatalog& catalog,
    const std::vector<std::string>& args);
std::optional<int> try_handle_selection_toolbox_create_batch_plan_catalog(
    const copperfin::localization::LocalizedCatalog& catalog,
    const std::vector<std::string>& args);
std::optional<int> try_handle_toolbox_create_batch_dispatch_catalog(
    const copperfin::localization::LocalizedCatalog& catalog,
    const std::vector<std::string>& args);
std::optional<int> try_handle_selection_toolbox_create_batch_dispatch_catalog(
    const copperfin::localization::LocalizedCatalog& catalog,
    const std::vector<std::string>& args);
std::optional<int> try_handle_toolbox_create_batch_plan(
    const copperfin::localization::LocalizedCatalog& catalog,
    const std::vector<std::string>& args);
std::optional<int> try_handle_selection_toolbox_create_batch_plan(
    const copperfin::localization::LocalizedCatalog& catalog,
    const std::vector<std::string>& args);
std::optional<int> try_handle_selection_toolbox_create_batch_dispatch_plan(
    const copperfin::localization::LocalizedCatalog& catalog,
    const std::vector<std::string>& args);
std::optional<int> try_handle_toolbox_create_batch_from_dispatch(
    const copperfin::localization::LocalizedCatalog& catalog,
    const std::vector<std::string>& args);
std::optional<int> try_handle_toolbox_create_batch_from_dispatch_plan(
    const copperfin::localization::LocalizedCatalog& catalog,
    const std::vector<std::string>& args);
std::optional<int> try_handle_toolbox_create_batch_dispatch_from_dispatch_plan(
    const copperfin::localization::LocalizedCatalog& catalog,
    const std::vector<std::string>& args);
std::optional<int> try_handle_toolbox_create_batch_dispatch_plan(
    const copperfin::localization::LocalizedCatalog& catalog,
    const std::vector<std::string>& args);
std::optional<int> try_handle_toolbox_create_batch(
    const copperfin::localization::LocalizedCatalog& catalog,
    const std::vector<std::string>& args);
std::optional<int> try_handle_selection_toolbox_create_batch(
    const copperfin::localization::LocalizedCatalog& catalog,
    const std::vector<std::string>& args);

// ==== Toolbox subsystem: dispatch, execution, and invocation-admission operations ====
std::string toolbox_parse_selection_batch_dispatch_item_requires_toolbox_item(
    const copperfin::localization::LocalizedCatalog& catalog);
ToolboxInvocationAdmissionParseResult parse_toolbox_invocation_admission_arguments(
    const copperfin::localization::LocalizedCatalog& catalog,
    const std::vector<std::string>& args);
ToolboxDispatchParseResult parse_toolbox_dispatch_arguments(
    const copperfin::localization::LocalizedCatalog& catalog,
    const std::vector<std::string>& args);
ToolboxExecuteParseResult parse_toolbox_execute_arguments(
    const copperfin::localization::LocalizedCatalog& catalog,
    const std::vector<std::string>& args);
ToolboxInvocationAdmissionCatalogParseResult parse_toolbox_invocation_admission_catalog_arguments(
    const copperfin::localization::LocalizedCatalog& catalog,
    const std::vector<std::string>& args);
SelectionToolboxInvocationAdmissionCatalogParseResult
parse_selection_toolbox_invocation_admission_catalog_arguments(
    const copperfin::localization::LocalizedCatalog& catalog,
    const std::vector<std::string>& args);
ToolboxDispatchCatalogParseResult parse_toolbox_dispatch_catalog_arguments(
    const copperfin::localization::LocalizedCatalog& catalog,
    const std::vector<std::string>& args);
ToolboxDispatchExecutionCatalogParseResult parse_toolbox_dispatch_execution_catalog_arguments(
    const copperfin::localization::LocalizedCatalog& catalog,
    const std::vector<std::string>& args);
SelectionToolboxDispatchExecutionCatalogParseResult
parse_selection_toolbox_dispatch_execution_catalog_arguments(
    const copperfin::localization::LocalizedCatalog& catalog,
    const std::vector<std::string>& args);
SelectionToolboxDispatchCatalogParseResult parse_selection_toolbox_dispatch_catalog_arguments(
    const copperfin::localization::LocalizedCatalog& catalog,
    const std::vector<std::string>& args);
void print_json_toolbox_invocation_admission_result(
    const copperfin::studio::StudioToolboxInvocationAdmissionResult& result);
void print_json_toolbox_dispatch_result(const copperfin::studio::StudioToolboxDispatchResult& result);
void print_json_toolbox_execution_result(
    const copperfin::studio::StudioToolboxDispatchExecutionResult& result,
    const std::string& launch_command,
    const std::string& executed_command);
void print_json_toolbox_invocation_admission_catalog_result(
    const copperfin::studio::StudioToolboxInvocationAdmissionCatalogResult& result);
void print_json_toolbox_dispatch_catalog_result(
    const copperfin::studio::StudioToolboxDispatchCatalogResult& result);
void print_json_toolbox_dispatch_execution_catalog_entry(
    const copperfin::studio::StudioToolboxDispatchExecutionCatalogEntry& entry,
    const std::string& indent);
void print_json_toolbox_dispatch_execution_catalog_result(
    const copperfin::studio::StudioToolboxDispatchExecutionCatalogResult& result);
void print_text_toolbox_invocation_admission_result(
    const copperfin::studio::StudioToolboxInvocationAdmissionResult& result);
void print_text_toolbox_invocation_admission_catalog_result(
    const copperfin::studio::StudioToolboxInvocationAdmissionCatalogResult& result);
void print_text_toolbox_dispatch_result(const copperfin::studio::StudioToolboxDispatchResult& result);
void print_text_toolbox_execution_result(
    const copperfin::studio::StudioToolboxDispatchExecutionResult& result,
    const std::string& launch_command,
    const std::string& executed_command);
void print_text_toolbox_dispatch_catalog_result(
    const copperfin::studio::StudioToolboxDispatchCatalogResult& result);
void print_text_toolbox_dispatch_execution_catalog_result(
    const copperfin::studio::StudioToolboxDispatchExecutionCatalogResult& result);
std::optional<int> try_handle_toolbox_invocation_admission(
    const copperfin::localization::LocalizedCatalog& catalog,
    const std::vector<std::string>& args);
std::optional<int> try_handle_toolbox_invocation_admission_catalog(
    const copperfin::localization::LocalizedCatalog& catalog,
    const std::vector<std::string>& args);
std::optional<int> try_handle_selection_toolbox_invocation_admission_catalog(
    const copperfin::localization::LocalizedCatalog& catalog,
    const std::vector<std::string>& args);
std::optional<int> try_handle_toolbox_dispatch(
    const copperfin::localization::LocalizedCatalog& catalog,
    const std::vector<std::string>& args);
std::optional<int> try_handle_toolbox_execute(
    const copperfin::localization::LocalizedCatalog& catalog,
    const std::vector<std::string>& args);
std::optional<int> try_handle_toolbox_dispatch_catalog(
    const copperfin::localization::LocalizedCatalog& catalog,
    const std::vector<std::string>& args);
std::optional<int> try_handle_toolbox_dispatch_execution_catalog(
    const copperfin::localization::LocalizedCatalog& catalog,
    const std::vector<std::string>& args);
std::optional<int> try_handle_selection_toolbox_dispatch_execution_catalog(
    const copperfin::localization::LocalizedCatalog& catalog,
    const std::vector<std::string>& args);
std::optional<int> try_handle_selection_toolbox_dispatch_catalog(
    const copperfin::localization::LocalizedCatalog& catalog,
    const std::vector<std::string>& args);

// ==== Toolbox subsystem: palette launch and query operations ====
std::string toolbox_palette_parse_missing_value(
    const copperfin::localization::LocalizedCatalog& catalog,
    const std::string& option);
std::string toolbox_palette_parse_unknown_selection_context_token(
    const copperfin::localization::LocalizedCatalog& catalog,
    const std::string& token);
std::string toolbox_palette_parse_unknown_toolbox_context_token(
    const copperfin::localization::LocalizedCatalog& catalog,
    const std::string& token);
std::string toolbox_palette_parse_non_negative_integer(
    const copperfin::localization::LocalizedCatalog& catalog,
    const std::string& option);
std::string toolbox_palette_parse_unknown_option(
    const copperfin::localization::LocalizedCatalog& catalog,
    const std::string& command_name,
    const std::string& argument);
std::string toolbox_palette_parse_message(
    const copperfin::localization::LocalizedCatalog& catalog,
    std::string_view key);
ToolboxPaletteLaunchPlanParseResult parse_toolbox_palette_launch_plan_arguments(
    const copperfin::localization::LocalizedCatalog& catalog,
    const std::vector<std::string>& args);
ToolboxPaletteLaunchCatalogParseResult parse_toolbox_palette_launch_catalog_arguments(
    const copperfin::localization::LocalizedCatalog& catalog,
    const std::vector<std::string>& args);
ToolboxPaletteQueryParseResult parse_toolbox_palette_query_arguments(
    const copperfin::localization::LocalizedCatalog& catalog,
    const std::vector<std::string>& args);
void print_json_toolbox_palette_query_result(
    const copperfin::studio::StudioToolboxPaletteQueryResult& result);
void print_json_toolbox_palette_launch_plan_result(
    const copperfin::studio::StudioToolboxPaletteLaunchPlanResult& result);
void print_json_toolbox_palette_launch_catalog_entry(
    const copperfin::studio::StudioToolboxPaletteLaunchCatalogEntry& entry,
    const std::string& indent);
void print_json_toolbox_palette_launch_catalog_result(
    const copperfin::studio::StudioToolboxPaletteLaunchCatalogResult& result);
void print_text_toolbox_palette_launch_plan_result(
    const copperfin::localization::LocalizedCatalog& catalog,
    const copperfin::studio::StudioToolboxPaletteLaunchPlanResult& result);
void print_text_toolbox_palette_launch_catalog_result(
    const copperfin::localization::LocalizedCatalog& catalog,
    const copperfin::studio::StudioToolboxPaletteLaunchCatalogResult& result);
void print_text_toolbox_palette_query_result(
    const copperfin::localization::LocalizedCatalog& catalog,
    const copperfin::studio::StudioToolboxPaletteQueryResult& result);
std::optional<int> try_handle_toolbox_palette_query(
    const copperfin::localization::LocalizedCatalog& catalog,
    const std::vector<std::string>& args);
std::optional<int> try_handle_toolbox_palette_launch(
    const copperfin::localization::LocalizedCatalog& catalog,
    const std::vector<std::string>& args);
std::optional<int> try_handle_toolbox_palette_launch_catalog(
    const copperfin::localization::LocalizedCatalog& catalog,
    const std::vector<std::string>& args);

// ==== Visual asset editor subsystem: method invocation operations ====
std::string visual_method_parse_missing_value(
    const copperfin::localization::LocalizedCatalog& catalog,
    const std::string& option);
std::string visual_method_parse_non_negative_integer(
    const copperfin::localization::LocalizedCatalog& catalog,
    const std::string& option);
std::string visual_method_parse_unknown_option(
    const copperfin::localization::LocalizedCatalog& catalog,
    const std::string& command_name,
    const std::string& argument);
std::string visual_method_parse_message(
    const copperfin::localization::LocalizedCatalog& catalog,
    std::string_view key);
VisualMethodListParseResult parse_visual_method_list_arguments(
    const copperfin::localization::LocalizedCatalog& catalog,
    const std::vector<std::string>& args);
VisualMethodQueryParseResult parse_visual_method_query_arguments(
    const copperfin::localization::LocalizedCatalog& catalog,
    const std::vector<std::string>& args);
VisualMethodUpdateParseResult parse_visual_method_update_arguments(
    const copperfin::localization::LocalizedCatalog& catalog,
    const std::vector<std::string>& args);
VisualMethodDeleteParseResult parse_visual_method_delete_arguments(
    const copperfin::localization::LocalizedCatalog& catalog,
    const std::vector<std::string>& args);
VisualMethodDeleteBatchParseResult parse_visual_method_delete_batch_arguments(
    const copperfin::localization::LocalizedCatalog& catalog,
    const std::vector<std::string>& args);
VisualMethodRenameParseResult parse_visual_method_rename_arguments(
    const copperfin::localization::LocalizedCatalog& catalog,
    const std::vector<std::string>& args);
VisualMethodRenameBatchParseResult parse_visual_method_rename_batch_arguments(
    const copperfin::localization::LocalizedCatalog& catalog,
    const std::vector<std::string>& args);
VisualMethodCopyParseResult parse_visual_method_copy_arguments(
    const copperfin::localization::LocalizedCatalog& catalog,
    const std::vector<std::string>& args);
VisualMethodCopyBatchParseResult parse_visual_method_copy_batch_arguments(
    const copperfin::localization::LocalizedCatalog& catalog,
    const std::vector<std::string>& args);
VisualMethodMoveBatchParseResult parse_visual_method_move_batch_arguments(
    const copperfin::localization::LocalizedCatalog& catalog,
    const std::vector<std::string>& args);
VisualMethodMoveParseResult parse_visual_method_move_arguments(
    const copperfin::localization::LocalizedCatalog& catalog,
    const std::vector<std::string>& args);
VisualMethodReorderParseResult parse_visual_method_reorder_arguments(
    const copperfin::localization::LocalizedCatalog& catalog,
    const std::vector<std::string>& args);
VisualMethodReorderBatchParseResult parse_visual_method_reorder_batch_arguments(
    const copperfin::localization::LocalizedCatalog& catalog,
    const std::vector<std::string>& args);
void print_json_visual_method_snapshot(const copperfin::vfp::VisualObjectMethodSnapshot& method,
                                       const std::string& indent);
void print_json_visual_method_list_result(
    const copperfin::vfp::VisualObjectMethodListResult& result);
void print_json_visual_method_query_result(
    const copperfin::vfp::VisualObjectMethodQueryResult& result);
void print_json_visual_method_update_result(
    const copperfin::vfp::VisualAssetEditResult& result,
    const copperfin::vfp::VisualAssetUndoStatus& undo_status,
    const std::string& result_name = "visualMethodUpdate",
    std::optional<bool> launched_from_visual_studio = std::nullopt);
void print_text_visual_method_list_result(
    const copperfin::vfp::VisualObjectMethodListResult& result);
void print_text_visual_method_query_result(
    const copperfin::vfp::VisualObjectMethodQueryResult& result);
void print_text_visual_method_update_result(
    const copperfin::vfp::VisualAssetEditResult& result,
    const copperfin::vfp::VisualAssetUndoStatus& undo_status);
std::optional<int> try_handle_visual_method_reorder_batch(
    const copperfin::localization::LocalizedCatalog& catalog,
    const std::vector<std::string>& args);
std::optional<int> try_handle_visual_method_reorder(
    const copperfin::localization::LocalizedCatalog& catalog,
    const std::vector<std::string>& args);
std::optional<int> try_handle_visual_method_delete_batch(
    const copperfin::localization::LocalizedCatalog& catalog,
    const std::vector<std::string>& args);
std::optional<int> try_handle_visual_method_rename_batch(
    const copperfin::localization::LocalizedCatalog& catalog,
    const std::vector<std::string>& args);
std::optional<int> try_handle_visual_method_copy_batch(
    const copperfin::localization::LocalizedCatalog& catalog,
    const std::vector<std::string>& args);
std::optional<int> try_handle_visual_method_move_batch(
    const copperfin::localization::LocalizedCatalog& catalog,
    const std::vector<std::string>& args);
std::optional<int> try_handle_visual_method_move(
    const copperfin::localization::LocalizedCatalog& catalog,
    const std::vector<std::string>& args);
std::optional<int> try_handle_visual_method_copy(
    const copperfin::localization::LocalizedCatalog& catalog,
    const std::vector<std::string>& args);
std::optional<int> try_handle_visual_method_rename(
    const copperfin::localization::LocalizedCatalog& catalog,
    const std::vector<std::string>& args);
std::optional<int> try_handle_visual_method_delete(
    const copperfin::localization::LocalizedCatalog& catalog,
    const std::vector<std::string>& args);
std::optional<int> try_handle_visual_method_update(
    const copperfin::localization::LocalizedCatalog& catalog,
    const std::vector<std::string>& args);
std::optional<int> try_handle_visual_method_query(
    const copperfin::localization::LocalizedCatalog& catalog,
    const std::vector<std::string>& args);
std::optional<int> try_handle_visual_method_list(
    const copperfin::localization::LocalizedCatalog& catalog,
    const std::vector<std::string>& args);

// ==== Visual asset editor subsystem: object lifecycle and hierarchy operations ====
std::string visual_object_parse_missing_value(
    const copperfin::localization::LocalizedCatalog& catalog,
    const std::string& option);
std::string visual_object_parse_non_negative_integer(
    const copperfin::localization::LocalizedCatalog& catalog,
    const std::string& option);
std::string visual_object_parse_unknown_option(
    const copperfin::localization::LocalizedCatalog& catalog,
    const std::string& command_name,
    const std::string& argument);
std::string visual_object_parse_message(
    const copperfin::localization::LocalizedCatalog& catalog,
    std::string_view key);
std::string visual_object_parse_subtree_replacement_requires_source(
    const copperfin::localization::LocalizedCatalog& catalog);
VisualObjectListParseResult parse_visual_object_list_arguments(
    const copperfin::localization::LocalizedCatalog& catalog,
    const std::vector<std::string>& args);
VisualObjectChildrenParseResult parse_visual_object_children_arguments(
    const copperfin::localization::LocalizedCatalog& catalog,
    const std::vector<std::string>& args);
VisualObjectDescendantsParseResult parse_visual_object_descendants_arguments(
    const copperfin::localization::LocalizedCatalog& catalog,
    const std::vector<std::string>& args);
VisualObjectAncestorsParseResult parse_visual_object_ancestors_arguments(
    const copperfin::localization::LocalizedCatalog& catalog,
    const std::vector<std::string>& args);
VisualObjectReparentBatchParseResult parse_visual_object_reparent_batch_arguments(
    const copperfin::localization::LocalizedCatalog& catalog,
    const std::vector<std::string>& args);
VisualObjectDuplicateBatchParseResult parse_visual_object_duplicate_batch_arguments(
    const copperfin::localization::LocalizedCatalog& catalog,
    const std::vector<std::string>& args);
VisualObjectDuplicateSubtreeParseResult parse_visual_object_duplicate_subtree_arguments(
    const copperfin::localization::LocalizedCatalog& catalog,
    const std::vector<std::string>& args);
VisualObjectRenameBatchParseResult parse_visual_object_rename_batch_arguments(
    const copperfin::localization::LocalizedCatalog& catalog,
    const std::vector<std::string>& args);
VisualObjectReorderBatchParseResult parse_visual_object_reorder_batch_arguments(
    const copperfin::localization::LocalizedCatalog& catalog,
    const std::vector<std::string>& args);
VisualObjectUpdateBatchParseResult parse_visual_object_update_batch_arguments(
    const copperfin::localization::LocalizedCatalog& catalog,
    const std::vector<std::string>& args);
void print_json_visual_object_snapshot(
    const copperfin::vfp::VisualObjectSnapshot& object,
    const std::string& indent);
void print_json_visual_object_list_result(
    const copperfin::vfp::VisualObjectListResult& result);
void print_json_visual_object_children_result(
    const copperfin::vfp::VisualObjectChildrenListResult& result);
void print_json_visual_object_descendants_result(
    const copperfin::vfp::VisualObjectDescendantsListResult& result);
void print_json_visual_object_ancestors_result(
    const copperfin::vfp::VisualObjectAncestorsListResult& result);
void print_json_visual_object_subtree_duplicate_result(
    const copperfin::vfp::VisualObjectSubtreeDuplicateResult& result,
    const copperfin::vfp::VisualAssetUndoStatus& undo_status);
void print_text_visual_object_list_result(
    const copperfin::vfp::VisualObjectListResult& result);
void print_text_visual_object_children_result(
    const copperfin::vfp::VisualObjectChildrenListResult& result);
void print_text_visual_object_descendants_result(
    const copperfin::vfp::VisualObjectDescendantsListResult& result);
void print_text_visual_object_ancestors_result(
    const copperfin::vfp::VisualObjectAncestorsListResult& result);
std::optional<int> try_handle_visual_object_duplicate_batch(
    const copperfin::localization::LocalizedCatalog& catalog,
    const std::vector<std::string>& args);
std::optional<int> try_handle_visual_object_duplicate_subtree(
    const copperfin::localization::LocalizedCatalog& catalog,
    const std::vector<std::string>& args);
std::optional<int> try_handle_visual_object_rename_batch(
    const copperfin::localization::LocalizedCatalog& catalog,
    const std::vector<std::string>& args);
std::optional<int> try_handle_visual_object_reorder_batch(
    const copperfin::localization::LocalizedCatalog& catalog,
    const std::vector<std::string>& args);
std::optional<int> try_handle_visual_object_reparent_batch(
    const copperfin::localization::LocalizedCatalog& catalog,
    const std::vector<std::string>& args);
std::optional<int> try_handle_visual_object_update_batch(
    const copperfin::localization::LocalizedCatalog& catalog,
    const std::vector<std::string>& args);
std::optional<int> try_handle_visual_object_ancestors(
    const copperfin::localization::LocalizedCatalog& catalog,
    const std::vector<std::string>& args);
std::optional<int> try_handle_visual_object_descendants(
    const copperfin::localization::LocalizedCatalog& catalog,
    const std::vector<std::string>& args);
std::optional<int> try_handle_visual_object_children(
    const copperfin::localization::LocalizedCatalog& catalog,
    const std::vector<std::string>& args);
std::optional<int> try_handle_visual_object_list(
    const copperfin::localization::LocalizedCatalog& catalog,
    const std::vector<std::string>& args);

// ==== Visual asset editor subsystem: property operations ====
std::string visual_property_parse_missing_value(
    const copperfin::localization::LocalizedCatalog& catalog,
    const std::string& option);
std::string visual_property_parse_non_negative_integer(
    const copperfin::localization::LocalizedCatalog& catalog,
    const std::string& option);
std::string visual_property_parse_boolean_value_required(
    const copperfin::localization::LocalizedCatalog& catalog,
    const std::string& option);
std::string visual_property_parse_unknown_option(
    const copperfin::localization::LocalizedCatalog& catalog,
    const std::string& command_name,
    const std::string& argument);
std::string visual_property_parse_batch_item_requires_property_name(
    const copperfin::localization::LocalizedCatalog& catalog,
    std::string_view key);
std::string visual_property_parse_message(
    const copperfin::localization::LocalizedCatalog& catalog,
    std::string_view key);
VisualPropertyFilterParseResult parse_visual_property_filter_arguments(
    const copperfin::localization::LocalizedCatalog& catalog,
    const std::vector<std::string>& args);
VisualPropertyQueryParseResult parse_visual_property_query_arguments(
    const copperfin::localization::LocalizedCatalog& catalog,
    const std::vector<std::string>& args);
VisualPropertyUpdateBatchParseResult parse_visual_property_update_batch_arguments(
    const copperfin::localization::LocalizedCatalog& catalog,
    const std::vector<std::string>& args);
VisualPropertyClearParseResult parse_visual_property_clear_arguments(
    const copperfin::localization::LocalizedCatalog& catalog,
    const std::vector<std::string>& args);
VisualPropertyClearBatchParseResult parse_visual_property_clear_batch_arguments(
    const copperfin::localization::LocalizedCatalog& catalog,
    const std::vector<std::string>& args);
VisualPropertyCopyParseResult parse_visual_property_copy_arguments(
    const copperfin::localization::LocalizedCatalog& catalog,
    const std::vector<std::string>& args);
VisualPropertyCopyBatchParseResult parse_visual_property_copy_batch_arguments(
    const copperfin::localization::LocalizedCatalog& catalog,
    const std::vector<std::string>& args);
VisualPropertyMoveParseResult parse_visual_property_move_arguments(
    const copperfin::localization::LocalizedCatalog& catalog,
    const std::vector<std::string>& args);
VisualPropertyMoveBatchParseResult parse_visual_property_move_batch_arguments(
    const copperfin::localization::LocalizedCatalog& catalog,
    const std::vector<std::string>& args);
VisualPropertyRenameParseResult parse_visual_property_rename_arguments(
    const copperfin::localization::LocalizedCatalog& catalog,
    const std::vector<std::string>& args);
VisualPropertyRenameBatchParseResult parse_visual_property_rename_batch_arguments(
    const copperfin::localization::LocalizedCatalog& catalog,
    const std::vector<std::string>& args);
VisualPropertyReorderParseResult parse_visual_property_reorder_arguments(
    const copperfin::localization::LocalizedCatalog& catalog,
    const std::vector<std::string>& args);
VisualPropertyReorderBatchParseResult parse_visual_property_reorder_batch_arguments(
    const copperfin::localization::LocalizedCatalog& catalog,
    const std::vector<std::string>& args);
VisualPropertyListParseResult parse_visual_property_list_arguments(
    const copperfin::localization::LocalizedCatalog& catalog,
    const std::vector<std::string>& args);
void print_json_visual_property_snapshot(
    const copperfin::vfp::VisualObjectPropertySnapshot& property,
    const std::string& indent);
void print_json_visual_property_query_result(
    const copperfin::vfp::VisualObjectPropertyQueryResult& result);
void print_json_visual_property_list_result(
    const copperfin::vfp::VisualObjectPropertyListResult& result);
void print_json_visual_property_filter_result(
    const copperfin::vfp::VisualObjectPropertyListFilterResult& result);
void print_text_visual_property_filter_result(
    const copperfin::vfp::VisualObjectPropertyListFilterResult& result);
void print_text_visual_property_query_result(
    const copperfin::vfp::VisualObjectPropertyQueryResult& result);
void print_text_visual_property_list_result(
    const copperfin::vfp::VisualObjectPropertyListResult& result);
std::optional<int> try_handle_visual_property_list(
    const copperfin::localization::LocalizedCatalog& catalog,
    const std::vector<std::string>& args);
std::optional<int> try_handle_visual_property_query(
    const copperfin::localization::LocalizedCatalog& catalog,
    const std::vector<std::string>& args);
std::optional<int> try_handle_visual_property_update_batch(
    const copperfin::localization::LocalizedCatalog& catalog,
    const std::vector<std::string>& args);
std::optional<int> try_handle_visual_property_clear(
    const copperfin::localization::LocalizedCatalog& catalog,
    const std::vector<std::string>& args);
std::optional<int> try_handle_visual_property_clear_batch(
    const copperfin::localization::LocalizedCatalog& catalog,
    const std::vector<std::string>& args);
std::optional<int> try_handle_visual_property_copy(
    const copperfin::localization::LocalizedCatalog& catalog,
    const std::vector<std::string>& args);
std::optional<int> try_handle_visual_property_copy_batch(
    const copperfin::localization::LocalizedCatalog& catalog,
    const std::vector<std::string>& args);
std::optional<int> try_handle_visual_property_move(
    const copperfin::localization::LocalizedCatalog& catalog,
    const std::vector<std::string>& args);
std::optional<int> try_handle_visual_property_move_batch(
    const copperfin::localization::LocalizedCatalog& catalog,
    const std::vector<std::string>& args);
std::optional<int> try_handle_visual_property_rename(
    const copperfin::localization::LocalizedCatalog& catalog,
    const std::vector<std::string>& args);
std::optional<int> try_handle_visual_property_rename_batch(
    const copperfin::localization::LocalizedCatalog& catalog,
    const std::vector<std::string>& args);
std::optional<int> try_handle_visual_property_reorder(
    const copperfin::localization::LocalizedCatalog& catalog,
    const std::vector<std::string>& args);
std::optional<int> try_handle_visual_property_reorder_batch(
    const copperfin::localization::LocalizedCatalog& catalog,
    const std::vector<std::string>& args);
std::optional<int> try_handle_visual_property_filter(
    const copperfin::localization::LocalizedCatalog& catalog,
    const std::vector<std::string>& args);

// ==== Selection-context and report-section resolution helpers shared across subsystems ====
void print_json_selection_toolbox_create_result(
    const copperfin::studio::StudioSelectionToolboxObjectCreateResult& result);
void print_json_selection_toolbox_create_plan_result(
    const copperfin::studio::StudioSelectionToolboxObjectCreatePlanResult& result);
void print_json_selection_toolbox_create_dispatch_plan_result(
    const copperfin::studio::StudioSelectionToolboxObjectCreateDispatchResult& result);
void print_json_selection_toolbox_create_batch_plan_result(
    const copperfin::studio::StudioSelectionToolboxObjectCreateBatchPlanResult& result);
void print_json_selection_toolbox_create_batch_result(
    const copperfin::studio::StudioSelectionToolboxObjectCreateBatchResult& result);
void print_json_selection_toolbox_create_batch_dispatch_plan_result(
    const copperfin::studio::StudioSelectionToolboxObjectCreateBatchDispatchResult& result);
void print_json_selection_toolbox_create_batch_dispatch_catalog_result(
    const copperfin::studio::StudioSelectionToolboxObjectCreateBatchDispatchCatalogResult& result);
void print_json_selection_toolbox_create_plan_catalog_result(
    const copperfin::studio::StudioSelectionToolboxObjectCreatePlanCatalogResult& result);
void print_json_selection_toolbox_create_batch_plan_catalog_result(
    const copperfin::studio::StudioSelectionToolboxObjectCreateBatchPlanCatalogResult& result);
void print_json_selection_toolbox_create_dispatch_catalog_result(
    const copperfin::studio::StudioSelectionToolboxObjectCreateDispatchCatalogResult& result);
void print_json_selection_builder_launch_catalog_entry(
    const copperfin::studio::StudioSelectionBuilderLaunchCatalogEntry& entry,
    const std::string& indent);
void print_json_selection_builder_launch_catalog_result(
    const copperfin::studio::StudioSelectionBuilderLaunchCatalogResult& result);
void print_json_selection_builder_invocation_admission_catalog_entry(
    const copperfin::studio::StudioSelectionBuilderInvocationAdmissionCatalogEntry& entry,
    const std::string& indent);
void print_json_selection_builder_invocation_admission_catalog_result(
    const copperfin::studio::StudioSelectionBuilderInvocationAdmissionCatalogResult& result);
void print_json_selection_builder_dispatch_catalog_entry(
    const copperfin::studio::StudioSelectionBuilderDispatchCatalogEntry& entry,
    const std::string& indent);
void print_json_selection_builder_dispatch_catalog_result(
    const copperfin::studio::StudioSelectionBuilderDispatchCatalogResult& result);
void print_json_selection_builder_dispatch_execution_catalog_entry(
    const copperfin::studio::StudioSelectionBuilderDispatchExecutionCatalogEntry& entry,
    const std::string& indent);
void print_json_selection_builder_dispatch_execution_catalog_result(
    const copperfin::studio::StudioSelectionBuilderDispatchExecutionCatalogResult& result);
void print_json_selection_toolbox_invocation_admission_catalog_result(
    const copperfin::studio::StudioSelectionToolboxInvocationAdmissionCatalogResult& result);
void print_json_selection_toolbox_dispatch_catalog_result(
    const copperfin::studio::StudioSelectionToolboxDispatchCatalogResult& result);
void print_json_selection_toolbox_dispatch_execution_catalog_result(
    const copperfin::studio::StudioSelectionToolboxDispatchExecutionCatalogResult& result);
void print_text_selection_toolbox_create_plan_result(
    const copperfin::studio::StudioSelectionToolboxObjectCreatePlanResult& result);
void print_text_selection_toolbox_create_result(
    const copperfin::studio::StudioSelectionToolboxObjectCreateResult& result);
void print_text_selection_toolbox_create_dispatch_plan_result(
    const copperfin::studio::StudioSelectionToolboxObjectCreateDispatchResult& result);
void print_text_selection_toolbox_create_batch_dispatch_plan_result(
    const copperfin::studio::StudioSelectionToolboxObjectCreateBatchDispatchResult& result);
void print_text_selection_toolbox_create_plan_catalog_result(
    const copperfin::studio::StudioSelectionToolboxObjectCreatePlanCatalogResult& result);
void print_text_selection_toolbox_create_batch_plan_catalog_result(
    const copperfin::studio::StudioSelectionToolboxObjectCreateBatchPlanCatalogResult& result);
void print_text_selection_toolbox_create_batch_plan_result(
    const copperfin::studio::StudioSelectionToolboxObjectCreateBatchPlanResult& result);
void print_text_selection_toolbox_create_batch_result(
    const copperfin::studio::StudioSelectionToolboxObjectCreateBatchResult& result);
void print_text_selection_toolbox_create_dispatch_catalog_result(
    const copperfin::studio::StudioSelectionToolboxObjectCreateDispatchCatalogResult& result);
void print_text_selection_toolbox_create_batch_dispatch_catalog_result(
    const copperfin::studio::StudioSelectionToolboxObjectCreateBatchDispatchCatalogResult& result);
void print_text_selection_builder_launch_catalog_result(
    const copperfin::studio::StudioSelectionBuilderLaunchCatalogResult& result);
void print_text_selection_builder_invocation_admission_catalog_result(
    const copperfin::studio::StudioSelectionBuilderInvocationAdmissionCatalogResult& result);
void print_text_selection_builder_dispatch_catalog_result(
    const copperfin::studio::StudioSelectionBuilderDispatchCatalogResult& result);
void print_text_selection_builder_dispatch_execution_catalog_result(
    const copperfin::studio::StudioSelectionBuilderDispatchExecutionCatalogResult& result);
void print_text_selection_toolbox_invocation_admission_catalog_result(
    const copperfin::studio::StudioSelectionToolboxInvocationAdmissionCatalogResult& result);
void print_text_selection_toolbox_dispatch_catalog_result(
    const copperfin::studio::StudioSelectionToolboxDispatchCatalogResult& result);
void print_text_selection_toolbox_dispatch_execution_catalog_result(
    const copperfin::studio::StudioSelectionToolboxDispatchExecutionCatalogResult& result);
void print_json_report_field_index_or_null(std::size_t field_index);
void print_json_report_line_index_or_null(std::size_t line_index);
void print_json_report_record_index_or_null(std::size_t record_index);
void print_json_report_named_values(
    const std::vector<copperfin::studio::StudioNamedValue>& values,
    const std::string& indent);
void print_json_report_kind_counts(
    const std::vector<copperfin::studio::StudioReportKindCount>& counts,
    const std::string& indent);
const copperfin::studio::StudioObjectSnapshot* find_selected_object(
    const std::vector<copperfin::studio::StudioObjectSnapshot>& objects,
    std::size_t record_index);
const copperfin::studio::StudioReportSectionSnapshot* find_selected_report_section(
    const copperfin::studio::StudioReportLayoutSnapshot& report_layout,
    std::size_t record_index);
const copperfin::studio::StudioLayoutObjectSnapshot* find_selected_report_object(
    const copperfin::studio::StudioReportLayoutSnapshot& report_layout,
    std::size_t record_index);
const copperfin::studio::StudioReportSectionSnapshot* find_selected_report_object_section(
    const copperfin::studio::StudioReportLayoutSnapshot& report_layout,
    std::size_t record_index);
std::vector<copperfin::studio::StudioNamedValue> find_selected_report_settings(
    const copperfin::studio::StudioReportLayoutSnapshot& report_layout,
    std::size_t record_index);
void print_json_report_layout_object(
    const copperfin::studio::StudioLayoutObjectSnapshot& object,
    const std::string& indent);
void print_json_report_layout_objects(
    const std::vector<copperfin::studio::StudioLayoutObjectSnapshot>& objects,
    const std::string& indent);
void print_json_report_layout_section(
    const copperfin::studio::StudioReportSectionSnapshot& section,
    const std::string& indent);
void print_json_report_layout_sections(
    const std::vector<copperfin::studio::StudioReportSectionSnapshot>& sections,
    const std::string& indent);
void print_json_report_layout_grouping(
    const copperfin::studio::StudioReportGroupingSnapshot& grouping,
    const std::string& indent);
void print_json_report_layout_groupings(
    const std::vector<copperfin::studio::StudioReportGroupingSnapshot>& groupings,
    const std::string& indent);

}  // namespace cf_studio_host_main_detail

#endif
