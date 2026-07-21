#include "test_toolbox_creation_support.h"
#include "test_toolbox_creation_tests.h"

int main()
{
    using namespace copperfin::toolbox_creation_tests;
    test_toolbox_creation_errors_resolve_through_localization_catalog();
    test_toolbox_creation_default_catalog_refreshes_when_locale_changes();
    test_toolbox_creation_planner_maps_descriptors_without_mutation();
    test_toolbox_creation_planner_respects_explicit_names_and_rejections();
    test_toolbox_creation_selection_planner_resolves_contexts_without_mutation();
    test_toolbox_creation_selection_create_executes_context_resolved_creates();
    test_toolbox_creation_selection_dispatch_planner_resolves_contexts_without_mutation();
    test_toolbox_creation_selection_batch_planner_resolves_contexts_without_mutation();
    test_toolbox_creation_selection_batch_create_executes_context_resolved_batches();
    test_toolbox_creation_selection_batch_dispatch_planner_resolves_contexts_without_mutation();
    test_toolbox_creation_planner_uses_admitted_palette_dispatch_without_mutation();
    test_toolbox_creation_planner_rejects_invalid_palette_dispatches_without_mutation();
    test_toolbox_creation_create_from_dispatch_executes_admitted_dispatches();
    test_toolbox_creation_batch_planner_uses_admitted_palette_dispatch_without_mutation();
    test_toolbox_creation_batch_planner_rejects_invalid_palette_dispatches_without_mutation();
    test_toolbox_creation_batch_create_from_dispatch_executes_admitted_dispatches();
    test_toolbox_creation_dispatch_planner_uses_admitted_palette_dispatch_without_mutation();
    test_toolbox_creation_dispatch_planner_rejects_invalid_palette_dispatches_without_stale_arguments();
    test_toolbox_creation_batch_dispatch_planner_uses_admitted_palette_dispatch_without_mutation();
    test_toolbox_creation_batch_dispatch_planner_rejects_invalid_palette_dispatches_without_stale_arguments();
    test_toolbox_creation_catalog_plans_form_and_report_contexts_without_mutation();
    test_toolbox_creation_dispatch_catalog_plans_context_dispatches_without_mutation();
    test_toolbox_creation_dispatch_catalog_reports_non_admitted_errors_without_stale_arguments();
    test_toolbox_creation_selection_dispatch_catalog_plans_context_dispatches_without_mutation();
    test_toolbox_creation_selection_dispatch_catalog_reports_non_admitted_errors_without_stale_arguments();
    test_toolbox_creation_batch_plan_catalog_plans_context_batches_without_mutation();
    test_toolbox_creation_batch_plan_catalog_reports_planning_errors_without_stale_plans();
    test_toolbox_creation_selection_batch_plan_catalog_plans_context_batches_without_mutation();
    test_toolbox_creation_batch_dispatch_catalog_plans_context_batches_without_mutation();
    test_toolbox_creation_batch_dispatch_catalog_reports_non_admitted_errors_without_stale_arguments();
    test_toolbox_creation_selection_batch_dispatch_catalog_plans_context_batches_without_mutation();
    test_toolbox_creation_selection_batch_dispatch_catalog_reports_non_admitted_errors_without_stale_arguments();
    test_toolbox_creation_batch_planner_reserves_names_without_mutation();
    test_toolbox_creation_batch_planner_rejects_invalid_batches_without_mutation();
    test_toolbox_creation_batch_create_maps_descriptors_and_metadata();
    test_toolbox_creation_batch_create_rejects_invalid_batches_without_partial_mutation();
    test_toolbox_creation_dispatch_plans_host_arguments_without_mutation();
    test_toolbox_creation_dispatch_rejects_invalid_plans_without_stale_arguments();
    test_toolbox_creation_batch_dispatch_plans_host_arguments_without_mutation();
    test_toolbox_creation_batch_dispatch_rejects_invalid_plans_without_stale_arguments();
    test_toolbox_creation_maps_descriptors_and_defaults();
    test_toolbox_creation_respects_explicit_object_name();
    test_toolbox_creation_rejects_unknown_toolbox_without_mutation();
    test_toolbox_creation_enforces_optional_context_filters();

    if (failures != 0)
    {
        std::cerr << failures << " test(s) failed.\n";
        return EXIT_FAILURE;
    }
    std::cout << "All tests passed.\n";
    return EXIT_SUCCESS;
}
