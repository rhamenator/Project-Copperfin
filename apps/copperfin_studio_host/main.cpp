// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.

#include "studio_host_main_support.h"

using namespace cf_studio_host_main_detail;

int run_studio_host_main(int argc, char** argv) {
    const auto catalog = load_localization(argc > 0 ? std::string_view(argv[0]) : std::string_view{});
    g_active_catalog = &catalog;
    g_executable_path = argc > 0 ? argv[0] : "";
    const auto hardening = copperfin::security::apply_default_process_hardening();
    if (!hardening.applied) {
        std::cerr << studio_warning_prefix() << hardening.message << "\n";
    }
    std::vector<std::string> args;
    args.reserve(argc > 1 ? static_cast<std::size_t>(argc - 1) : 0U);
    for (int index = 1; index < argc; ++index) {
        args.emplace_back(argv[index]);
    }

    if (const auto handled = try_handle_license_status(catalog, args)) {
        return *handled;
    }
    if (const auto handled = try_handle_list_subsystems(catalog, args)) {
        return *handled;
    }
    if (const auto handled = try_handle_rushmore_explain(catalog, args)) {
        return *handled;
    }
    if (const auto handled = try_handle_builder_launch(catalog, args)) {
        return *handled;
    }
    if (const auto handled = try_handle_builder_launch_catalog(catalog, args)) {
        return *handled;
    }
    if (const auto handled = try_handle_selection_builder_launch_catalog(catalog, args)) {
        return *handled;
    }
    if (const auto handled = try_handle_builder_invocation_admission(catalog, args)) {
        return *handled;
    }
    if (const auto handled = try_handle_builder_invocation_admission_catalog(catalog, args)) {
        return *handled;
    }
    if (const auto handled = try_handle_selection_builder_invocation_admission_catalog(catalog, args)) {
        return *handled;
    }
    if (const auto handled = try_handle_builder_dispatch(catalog, args)) {
        return *handled;
    }
    if (const auto handled = try_handle_builder_execute(catalog, args)) {
        return *handled;
    }
    if (const auto handled = try_handle_builder_dispatch_catalog(catalog, args)) {
        return *handled;
    }
    if (const auto handled = try_handle_builder_dispatch_execution_catalog(catalog, args)) {
        return *handled;
    }
    if (const auto handled = try_handle_selection_builder_dispatch_execution_catalog(catalog, args)) {
        return *handled;
    }
    if (const auto handled = try_handle_selection_builder_dispatch_catalog(catalog, args)) {
        return *handled;
    }
    if (const auto handled = try_handle_editor_action_launch(catalog, args)) {
        return *handled;
    }
    if (const auto handled = try_handle_editor_action_launch_catalog(catalog, args)) {
        return *handled;
    }
    if (const auto handled = try_handle_editor_action_invocation_admission(catalog, args)) {
        return *handled;
    }
    if (const auto handled = try_handle_editor_action_invocation_admission_catalog(catalog, args)) {
        return *handled;
    }
    if (const auto handled = try_handle_editor_action_dispatch(catalog, args)) {
        return *handled;
    }
    if (const auto handled = try_handle_editor_action_execute(catalog, args)) {
        return *handled;
    }
    if (const auto handled = try_handle_editor_action_dispatch_catalog(catalog, args)) {
        return *handled;
    }
    if (const auto handled = try_handle_editor_action_dispatch_execution_catalog(catalog, args)) {
        return *handled;
    }
    if (const auto handled = try_handle_visual_method_reorder_batch(catalog, args)) {
        return *handled;
    }
    if (const auto handled = try_handle_visual_method_reorder(catalog, args)) {
        return *handled;
    }
    if (const auto handled = try_handle_visual_method_delete_batch(catalog, args)) {
        return *handled;
    }
    if (const auto handled = try_handle_visual_method_rename_batch(catalog, args)) {
        return *handled;
    }
    if (const auto handled = try_handle_visual_method_copy_batch(catalog, args)) {
        return *handled;
    }
    if (const auto handled = try_handle_visual_method_move_batch(catalog, args)) {
        return *handled;
    }
    if (const auto handled = try_handle_visual_method_move(catalog, args)) {
        return *handled;
    }
    if (const auto handled = try_handle_visual_method_copy(catalog, args)) {
        return *handled;
    }
    if (const auto handled = try_handle_visual_method_rename(catalog, args)) {
        return *handled;
    }
    if (const auto handled = try_handle_visual_method_delete(catalog, args)) {
        return *handled;
    }
    if (const auto handled = try_handle_visual_method_update(catalog, args)) {
        return *handled;
    }
    if (const auto handled = try_handle_visual_method_query(catalog, args)) {
        return *handled;
    }
    if (const auto handled = try_handle_visual_method_list(catalog, args)) {
        return *handled;
    }
    if (const auto handled = try_handle_visual_object_duplicate_batch(catalog, args)) {
        return *handled;
    }
    if (const auto handled = try_handle_visual_object_duplicate_subtree(catalog, args)) {
        return *handled;
    }
    if (const auto handled = try_handle_visual_object_rename_batch(catalog, args)) {
        return *handled;
    }
    if (const auto handled = try_handle_visual_object_reorder_batch(catalog, args)) {
        return *handled;
    }
    if (const auto handled = try_handle_visual_object_reparent_batch(catalog, args)) {
        return *handled;
    }
    if (const auto handled = try_handle_visual_object_update_batch(catalog, args)) {
        return *handled;
    }
    if (const auto handled = try_handle_visual_object_ancestors(catalog, args)) {
        return *handled;
    }
    if (const auto handled = try_handle_visual_object_descendants(catalog, args)) {
        return *handled;
    }
    if (const auto handled = try_handle_visual_object_children(catalog, args)) {
        return *handled;
    }
    if (const auto handled = try_handle_visual_object_list(catalog, args)) {
        return *handled;
    }
    if (const auto handled = try_handle_visual_property_list(catalog, args)) {
        return *handled;
    }
    if (const auto handled = try_handle_visual_property_query(catalog, args)) {
        return *handled;
    }
    if (const auto handled = try_handle_visual_property_update_batch(catalog, args)) {
        return *handled;
    }
    if (const auto handled = try_handle_visual_property_clear(catalog, args)) {
        return *handled;
    }
    if (const auto handled = try_handle_visual_property_clear_batch(catalog, args)) {
        return *handled;
    }
    if (const auto handled = try_handle_visual_property_copy(catalog, args)) {
        return *handled;
    }
    if (const auto handled = try_handle_visual_property_copy_batch(catalog, args)) {
        return *handled;
    }
    if (const auto handled = try_handle_visual_property_move(catalog, args)) {
        return *handled;
    }
    if (const auto handled = try_handle_visual_property_move_batch(catalog, args)) {
        return *handled;
    }
    if (const auto handled = try_handle_visual_property_rename(catalog, args)) {
        return *handled;
    }
    if (const auto handled = try_handle_visual_property_rename_batch(catalog, args)) {
        return *handled;
    }
    if (const auto handled = try_handle_visual_property_reorder(catalog, args)) {
        return *handled;
    }
    if (const auto handled = try_handle_visual_property_reorder_batch(catalog, args)) {
        return *handled;
    }
    if (const auto handled = try_handle_visual_property_filter(catalog, args)) {
        return *handled;
    }
    if (const auto handled = try_handle_toolbox_palette_query(catalog, args)) {
        return *handled;
    }
    if (const auto handled = try_handle_toolbox_palette_launch(catalog, args)) {
        return *handled;
    }
    if (const auto handled = try_handle_toolbox_palette_launch_catalog(catalog, args)) {
        return *handled;
    }
    if (const auto handled = try_handle_toolbox_invocation_admission(catalog, args)) {
        return *handled;
    }
    if (const auto handled = try_handle_toolbox_invocation_admission_catalog(catalog, args)) {
        return *handled;
    }
    if (const auto handled = try_handle_selection_toolbox_invocation_admission_catalog(catalog, args)) {
        return *handled;
    }
    if (const auto handled = try_handle_toolbox_dispatch(catalog, args)) {
        return *handled;
    }
    if (const auto handled = try_handle_toolbox_execute(catalog, args)) {
        return *handled;
    }
    if (const auto handled = try_handle_toolbox_dispatch_catalog(catalog, args)) {
        return *handled;
    }
    if (const auto handled = try_handle_toolbox_dispatch_execution_catalog(catalog, args)) {
        return *handled;
    }
    if (const auto handled = try_handle_selection_toolbox_dispatch_execution_catalog(catalog, args)) {
        return *handled;
    }
    if (const auto handled = try_handle_selection_toolbox_dispatch_catalog(catalog, args)) {
        return *handled;
    }
    if (const auto handled = try_handle_designer_launch_surfaces(catalog, args)) {
        return *handled;
    }
    if (const auto handled = try_handle_designer_invocation_admission(catalog, args)) {
        return *handled;
    }
    if (const auto handled = try_handle_designer_dispatch(catalog, args)) {
        return *handled;
    }
    if (const auto handled = try_handle_designer_execute(catalog, args)) {
        return *handled;
    }
    if (const auto handled = try_handle_designer_dispatch_execution_catalog(catalog, args)) {
        return *handled;
    }
    if (const auto handled = try_handle_designer_dispatch_catalog(catalog, args)) {
        return *handled;
    }
    if (const auto handled = try_handle_designer_invocation_admission_catalog(catalog, args)) {
        return *handled;
    }
    if (const auto handled = try_handle_designer_launch_surface_catalog(catalog, args)) {
        return *handled;
    }
    if (const auto handled = try_handle_toolbox_create_batch_plan_catalog(catalog, args)) {
        return *handled;
    }
    if (const auto handled = try_handle_selection_toolbox_create_batch_plan_catalog(catalog, args)) {
        return *handled;
    }
    if (const auto handled = try_handle_toolbox_create_batch_dispatch_catalog(catalog, args)) {
        return *handled;
    }
    if (const auto handled = try_handle_selection_toolbox_create_batch_dispatch_catalog(catalog, args)) {
        return *handled;
    }
    if (const auto handled = try_handle_toolbox_create_dispatch_catalog(catalog, args)) {
        return *handled;
    }
    if (const auto handled = try_handle_selection_toolbox_create_dispatch_catalog(catalog, args)) {
        return *handled;
    }
    if (const auto handled = try_handle_toolbox_create_plan_catalog(catalog, args)) {
        return *handled;
    }
    if (const auto handled = try_handle_selection_toolbox_create_plan_catalog(catalog, args)) {
        return *handled;
    }
    if (const auto handled = try_handle_toolbox_create_batch_plan(catalog, args)) {
        return *handled;
    }
    if (const auto handled = try_handle_selection_toolbox_create_batch_plan(catalog, args)) {
        return *handled;
    }
    if (const auto handled = try_handle_selection_toolbox_create_batch_dispatch_plan(catalog, args)) {
        return *handled;
    }
    if (const auto handled = try_handle_toolbox_create_batch_from_dispatch(catalog, args)) {
        return *handled;
    }
    if (const auto handled = try_handle_toolbox_create_batch_from_dispatch_plan(catalog, args)) {
        return *handled;
    }
    if (const auto handled = try_handle_toolbox_create_batch_dispatch_from_dispatch_plan(catalog, args)) {
        return *handled;
    }
    if (const auto handled = try_handle_toolbox_create_batch_dispatch_plan(catalog, args)) {
        return *handled;
    }
    if (const auto handled = try_handle_toolbox_create_from_dispatch(catalog, args)) {
        return *handled;
    }
    if (const auto handled = try_handle_toolbox_create_from_dispatch_plan(catalog, args)) {
        return *handled;
    }
    if (const auto handled = try_handle_toolbox_create_dispatch_from_dispatch_plan(catalog, args)) {
        return *handled;
    }
    if (const auto handled = try_handle_toolbox_create_dispatch_plan(catalog, args)) {
        return *handled;
    }
    if (const auto handled = try_handle_selection_toolbox_create_dispatch_plan(catalog, args)) {
        return *handled;
    }
    if (const auto handled = try_handle_toolbox_create_batch(catalog, args)) {
        return *handled;
    }
    if (const auto handled = try_handle_selection_toolbox_create_batch(catalog, args)) {
        return *handled;
    }
    if (const auto handled = try_handle_selection_toolbox_create_plan(catalog, args)) {
        return *handled;
    }
    if (const auto handled = try_handle_toolbox_create_plan(catalog, args)) {
        return *handled;
    }
    if (const auto handled = try_handle_selection_toolbox_create(catalog, args)) {
        return *handled;
    }
    if (const auto handled = try_handle_toolbox_create(catalog, args)) {
        return *handled;
    }

    const auto parse_result = copperfin::studio::parse_launch_arguments(args, catalog);
    if (!parse_result.ok) {
        std::cout << "status: error\n";
        std::cout << studio_error_prefix() << parse_result.error << "\n";
        print_usage(catalog);
        return 2;
    }
    if (parse_result.show_help) {
        print_usage(catalog);
        return 0;
    }
    auto open_request = parse_result.request;
    const auto select_open_request_visual_object = [&]() {
        const auto objects = copperfin::vfp::list_visual_objects(parse_result.request.path);
        if (!objects.ok) {
            return;
        }
        const auto object = std::find_if(
            objects.objects.begin(),
            objects.objects.end(),
            [&](const copperfin::vfp::VisualObjectSnapshot& candidate) {
                if (!open_request.unique_id.empty()) {
                    return candidate.unique_id == open_request.unique_id;
                }
                if (!open_request.object_name.empty()) {
                    return candidate.object_name == open_request.object_name;
                }
                return candidate.record_index == open_request.record_index;
            });
        if (object != objects.objects.end()) {
            open_request.record_index = object->record_index;
            open_request.selection_record_available = true;
        }
    };
    if (parse_result.request.undo_mode == copperfin::studio::StudioUndoMode::command) {
        const auto undo_result = copperfin::vfp::undo_visual_object_property(parse_result.request.path);
        if (!undo_result.ok) {
            std::cout << "status: error\n";
            std::cout << studio_error_prefix() << undo_result.error << "\n";
            return 5;
        }
    }
    bool asset_mutation_performed = parse_result.mutates_asset;
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
            std::cout << studio_error_prefix() << update_result.error << "\n";
            return 4;
        }

        asset_mutation_performed = true;
        select_open_request_visual_object();
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
            std::cout << studio_error_prefix() << clear_result.error << "\n";
            return 4;
        }

        asset_mutation_performed = true;
        select_open_request_visual_object();
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
            std::cout << studio_error_prefix() << rename_result.error << "\n";
            return 4;
        }

        asset_mutation_performed = true;
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
            std::cout << studio_error_prefix() << delete_result.error << "\n";
            return 4;
        }

        asset_mutation_performed = true;
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
            std::cout << studio_error_prefix() << restore_result.error << "\n";
            return 4;
        }

        asset_mutation_performed = true;
    }
    if (parse_result.request.deleted_states) {
        std::vector<copperfin::vfp::VisualObjectDeletedStateBatchItem> deleted_state_objects;
        deleted_state_objects.reserve(parse_result.request.deleted_state_objects.size());
        for (const auto& deleted_state_object : parse_result.request.deleted_state_objects) {
            deleted_state_objects.push_back({
                .record_index = deleted_state_object.record_index,
                .object_name = deleted_state_object.object_name,
                .unique_id = deleted_state_object.unique_id,
                .deleted = deleted_state_object.deleted
            });
        }

        const auto deleted_states_result = copperfin::vfp::set_visual_object_deleted_states({
            .path = parse_result.request.path,
            .objects = deleted_state_objects
        });

        if (!deleted_states_result.ok) {
            std::cout << "status: error\n";
            std::cout << studio_error_prefix() << deleted_states_result.error << "\n";
            return 4;
        }
    }
    if (parse_result.request.subtree_deleted_state) {
        const auto subtree_result = copperfin::vfp::set_visual_object_subtree_deleted_state({
            .path = parse_result.request.path,
            .record_index = parse_result.request.record_index,
            .object_name = parse_result.request.object_name,
            .unique_id = parse_result.request.unique_id,
            .deleted = parse_result.request.subtree_deleted
        });

        if (!subtree_result.ok) {
            std::cout << "status: error\n";
            std::cout << studio_error_prefix() << subtree_result.error << "\n";
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
            std::cout << studio_error_prefix() << duplicate_result.error << "\n";
            return 4;
        }

        open_request.record_index = duplicate_result.record_index;
        open_request.object_name = duplicate_result.object_name;
        open_request.unique_id = duplicate_result.unique_id;
        open_request.selection_record_available = true;
        asset_mutation_performed = true;
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
            std::cout << studio_error_prefix() << rename_result.error << "\n";
            return 4;
        }

        if (!parse_result.request.new_object_name.empty()) {
            open_request.object_name = parse_result.request.new_object_name;
        }
        if (!parse_result.request.new_unique_id.empty()) {
            open_request.unique_id = parse_result.request.new_unique_id;
        }
        select_open_request_visual_object();
        asset_mutation_performed = true;
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
            std::cout << studio_error_prefix() << reparent_result.error << "\n";
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
            std::cout << studio_error_prefix() << reorder_result.error << "\n";
            return 4;
        }

        select_open_request_visual_object();
        asset_mutation_performed = true;
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
            std::cout << studio_error_prefix() << group_result.error << "\n";
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
            std::cout << studio_error_prefix() << align_result.error << "\n";
            return 4;
        }

        select_open_request_visual_object();
        asset_mutation_performed = true;
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
            std::cout << studio_error_prefix() << resize_result.error << "\n";
            return 4;
        }

        select_open_request_visual_object();
        asset_mutation_performed = true;
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
            std::cout << studio_error_prefix() << distribute_result.error << "\n";
            return 4;
        }

        select_open_request_visual_object();
        asset_mutation_performed = true;
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
            std::cout << studio_error_prefix() << snap_result.error << "\n";
            return 4;
        }

        select_open_request_visual_object();
        asset_mutation_performed = true;
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
            std::cout << studio_error_prefix() << nudge_result.error << "\n";
            return 4;
        }

        select_open_request_visual_object();
        asset_mutation_performed = true;
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
            std::cout << studio_error_prefix() << tab_order_result.error << "\n";
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
            std::cout << studio_error_prefix() << tab_stop_result.error << "\n";
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
            std::cout << studio_error_prefix() << visibility_result.error << "\n";
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
            std::cout << studio_error_prefix() << enabled_result.error << "\n";
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
            std::cout << studio_error_prefix() << read_only_result.error << "\n";
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
            std::cout << studio_error_prefix() << locked_result.error << "\n";
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
            std::cout << studio_error_prefix() << caption_result.error << "\n";
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
            std::cout << studio_error_prefix() << picture_result.error << "\n";
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
            std::cout << studio_error_prefix() << down_picture_result.error << "\n";
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
            std::cout << studio_error_prefix() << disabled_picture_result.error << "\n";
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
            std::cout << studio_error_prefix() << ole_drag_picture_result.error << "\n";
            return 4;
        }
    }
    if (parse_result.request.mouse_icon_object) {
        std::vector<copperfin::vfp::VisualObjectAlignmentTarget> mouse_icon_objects;
        mouse_icon_objects.reserve(parse_result.request.mouse_icon_objects.size());
        for (const auto& mouse_icon_object : parse_result.request.mouse_icon_objects) {
            mouse_icon_objects.push_back({
                .record_index = mouse_icon_object.record_index,
                .object_name = mouse_icon_object.object_name,
                .unique_id = mouse_icon_object.unique_id
            });
        }

        const auto mouse_icon_result = copperfin::vfp::set_visual_object_mouse_icon({
            .path = parse_result.request.path,
            .objects = mouse_icon_objects,
            .mouse_icon = parse_result.request.mouse_icon
        });

        if (!mouse_icon_result.ok) {
            std::cout << "status: error\n";
            std::cout << studio_error_prefix() << mouse_icon_result.error << "\n";
            return 4;
        }
    }
    if (parse_result.request.drag_icon_object) {
        std::vector<copperfin::vfp::VisualObjectAlignmentTarget> drag_icon_objects;
        drag_icon_objects.reserve(parse_result.request.drag_icon_objects.size());
        for (const auto& drag_icon_object : parse_result.request.drag_icon_objects) {
            drag_icon_objects.push_back({
                .record_index = drag_icon_object.record_index,
                .object_name = drag_icon_object.object_name,
                .unique_id = drag_icon_object.unique_id
            });
        }

        const auto drag_icon_result = copperfin::vfp::set_visual_object_drag_icon({
            .path = parse_result.request.path,
            .objects = drag_icon_objects,
            .drag_icon = parse_result.request.drag_icon
        });

        if (!drag_icon_result.ok) {
            std::cout << "status: error\n";
            std::cout << studio_error_prefix() << drag_icon_result.error << "\n";
            return 4;
        }
    }
    if (parse_result.request.drag_mode_object) {
        std::vector<copperfin::vfp::VisualObjectAlignmentTarget> drag_mode_objects;
        drag_mode_objects.reserve(parse_result.request.drag_mode_objects.size());
        for (const auto& drag_mode_object : parse_result.request.drag_mode_objects) {
            drag_mode_objects.push_back({
                .record_index = drag_mode_object.record_index,
                .object_name = drag_mode_object.object_name,
                .unique_id = drag_mode_object.unique_id
            });
        }

        const auto drag_mode_result = copperfin::vfp::set_visual_object_drag_mode({
            .path = parse_result.request.path,
            .objects = drag_mode_objects,
            .drag_mode = parse_result.request.drag_mode
        });

        if (!drag_mode_result.ok) {
            std::cout << "status: error\n";
            std::cout << studio_error_prefix() << drag_mode_result.error << "\n";
            return 4;
        }
    }
    if (parse_result.request.ole_drag_mode_object) {
        std::vector<copperfin::vfp::VisualObjectAlignmentTarget> ole_drag_mode_objects;
        ole_drag_mode_objects.reserve(parse_result.request.ole_drag_mode_objects.size());
        for (const auto& ole_drag_mode_object : parse_result.request.ole_drag_mode_objects) {
            ole_drag_mode_objects.push_back({
                .record_index = ole_drag_mode_object.record_index,
                .object_name = ole_drag_mode_object.object_name,
                .unique_id = ole_drag_mode_object.unique_id
            });
        }

        const auto ole_drag_mode_result = copperfin::vfp::set_visual_object_ole_drag_mode({
            .path = parse_result.request.path,
            .objects = ole_drag_mode_objects,
            .ole_drag_mode = parse_result.request.ole_drag_mode
        });

        if (!ole_drag_mode_result.ok) {
            std::cout << "status: error\n";
            std::cout << studio_error_prefix() << ole_drag_mode_result.error << "\n";
            return 4;
        }
    }
    if (parse_result.request.ole_drop_mode_object) {
        std::vector<copperfin::vfp::VisualObjectAlignmentTarget> ole_drop_mode_objects;
        ole_drop_mode_objects.reserve(parse_result.request.ole_drop_mode_objects.size());
        for (const auto& ole_drop_mode_object : parse_result.request.ole_drop_mode_objects) {
            ole_drop_mode_objects.push_back({
                .record_index = ole_drop_mode_object.record_index,
                .object_name = ole_drop_mode_object.object_name,
                .unique_id = ole_drop_mode_object.unique_id
            });
        }

        const auto ole_drop_mode_result = copperfin::vfp::set_visual_object_ole_drop_mode({
            .path = parse_result.request.path,
            .objects = ole_drop_mode_objects,
            .ole_drop_mode = parse_result.request.ole_drop_mode
        });

        if (!ole_drop_mode_result.ok) {
            std::cout << "status: error\n";
            std::cout << studio_error_prefix() << ole_drop_mode_result.error << "\n";
            return 4;
        }
    }
    if (parse_result.request.ole_drop_effects_object) {
        std::vector<copperfin::vfp::VisualObjectAlignmentTarget> ole_drop_effects_objects;
        ole_drop_effects_objects.reserve(parse_result.request.ole_drop_effects_objects.size());
        for (const auto& ole_drop_effects_object : parse_result.request.ole_drop_effects_objects) {
            ole_drop_effects_objects.push_back({
                .record_index = ole_drop_effects_object.record_index,
                .object_name = ole_drop_effects_object.object_name,
                .unique_id = ole_drop_effects_object.unique_id
            });
        }

        const auto ole_drop_effects_result = copperfin::vfp::set_visual_object_ole_drop_effects({
            .path = parse_result.request.path,
            .objects = ole_drop_effects_objects,
            .ole_drop_effects = parse_result.request.ole_drop_effects
        });

        if (!ole_drop_effects_result.ok) {
            std::cout << "status: error\n";
            std::cout << studio_error_prefix() << ole_drop_effects_result.error << "\n";
            return 4;
        }
    }
    if (parse_result.request.ole_drop_text_insertion_object) {
        std::vector<copperfin::vfp::VisualObjectAlignmentTarget> ole_drop_text_insertion_objects;
        ole_drop_text_insertion_objects.reserve(parse_result.request.ole_drop_text_insertion_objects.size());
        for (const auto& ole_drop_text_insertion_object : parse_result.request.ole_drop_text_insertion_objects) {
            ole_drop_text_insertion_objects.push_back({
                .record_index = ole_drop_text_insertion_object.record_index,
                .object_name = ole_drop_text_insertion_object.object_name,
                .unique_id = ole_drop_text_insertion_object.unique_id
            });
        }

        const auto ole_drop_text_insertion_result = copperfin::vfp::set_visual_object_ole_drop_text_insertion({
            .path = parse_result.request.path,
            .objects = ole_drop_text_insertion_objects,
            .ole_drop_text_insertion = parse_result.request.ole_drop_text_insertion
        });

        if (!ole_drop_text_insertion_result.ok) {
            std::cout << "status: error\n";
            std::cout << studio_error_prefix() << ole_drop_text_insertion_result.error << "\n";
            return 4;
        }
    }
    if (parse_result.request.button_count_object) {
        std::vector<copperfin::vfp::VisualObjectAlignmentTarget> button_count_objects;
        button_count_objects.reserve(parse_result.request.button_count_objects.size());
        for (const auto& button_count_object : parse_result.request.button_count_objects) {
            button_count_objects.push_back({
                .record_index = button_count_object.record_index,
                .object_name = button_count_object.object_name,
                .unique_id = button_count_object.unique_id
            });
        }

        const auto button_count_result = copperfin::vfp::set_visual_object_button_count({
            .path = parse_result.request.path,
            .objects = button_count_objects,
            .button_count = parse_result.request.button_count
        });

        if (!button_count_result.ok) {
            std::cout << "status: error\n";
            std::cout << studio_error_prefix() << button_count_result.error << "\n";
            return 4;
        }
    }
    if (parse_result.request.curvature_object) {
        std::vector<copperfin::vfp::VisualObjectAlignmentTarget> curvature_objects;
        curvature_objects.reserve(parse_result.request.curvature_objects.size());
        for (const auto& curvature_object : parse_result.request.curvature_objects) {
            curvature_objects.push_back({
                .record_index = curvature_object.record_index,
                .object_name = curvature_object.object_name,
                .unique_id = curvature_object.unique_id
            });
        }

        const auto curvature_result = copperfin::vfp::set_visual_object_curvature({
            .path = parse_result.request.path,
            .objects = curvature_objects,
            .curvature = parse_result.request.curvature
        });

        if (!curvature_result.ok) {
            std::cout << "status: error\n";
            std::cout << studio_error_prefix() << curvature_result.error << "\n";
            return 4;
        }
    }
    if (parse_result.request.draw_mode_object) {
        std::vector<copperfin::vfp::VisualObjectAlignmentTarget> draw_mode_objects;
        draw_mode_objects.reserve(parse_result.request.draw_mode_objects.size());
        for (const auto& draw_mode_object : parse_result.request.draw_mode_objects) {
            draw_mode_objects.push_back({
                .record_index = draw_mode_object.record_index,
                .object_name = draw_mode_object.object_name,
                .unique_id = draw_mode_object.unique_id
            });
        }

        const auto draw_mode_result = copperfin::vfp::set_visual_object_draw_mode({
            .path = parse_result.request.path,
            .objects = draw_mode_objects,
            .draw_mode = parse_result.request.draw_mode
        });

        if (!draw_mode_result.ok) {
            std::cout << "status: error\n";
            std::cout << studio_error_prefix() << draw_mode_result.error << "\n";
            return 4;
        }
    }
    if (parse_result.request.draw_style_object) {
        std::vector<copperfin::vfp::VisualObjectAlignmentTarget> draw_style_objects;
        draw_style_objects.reserve(parse_result.request.draw_style_objects.size());
        for (const auto& draw_style_object : parse_result.request.draw_style_objects) {
            draw_style_objects.push_back({
                .record_index = draw_style_object.record_index,
                .object_name = draw_style_object.object_name,
                .unique_id = draw_style_object.unique_id
            });
        }

        const auto draw_style_result = copperfin::vfp::set_visual_object_draw_style({
            .path = parse_result.request.path,
            .objects = draw_style_objects,
            .draw_style = parse_result.request.draw_style
        });

        if (!draw_style_result.ok) {
            std::cout << "status: error\n";
            std::cout << studio_error_prefix() << draw_style_result.error << "\n";
            return 4;
        }
    }
    if (parse_result.request.draw_width_object) {
        std::vector<copperfin::vfp::VisualObjectAlignmentTarget> draw_width_objects;
        draw_width_objects.reserve(parse_result.request.draw_width_objects.size());
        for (const auto& draw_width_object : parse_result.request.draw_width_objects) {
            draw_width_objects.push_back({
                .record_index = draw_width_object.record_index,
                .object_name = draw_width_object.object_name,
                .unique_id = draw_width_object.unique_id
            });
        }

        const auto draw_width_result = copperfin::vfp::set_visual_object_draw_width({
            .path = parse_result.request.path,
            .objects = draw_width_objects,
            .draw_width = parse_result.request.draw_width
        });

        if (!draw_width_result.ok) {
            std::cout << "status: error\n";
            std::cout << studio_error_prefix() << draw_width_result.error << "\n";
            return 4;
        }
    }
    if (parse_result.request.fill_style_object) {
        std::vector<copperfin::vfp::VisualObjectAlignmentTarget> fill_style_objects;
        fill_style_objects.reserve(parse_result.request.fill_style_objects.size());
        for (const auto& fill_style_object : parse_result.request.fill_style_objects) {
            fill_style_objects.push_back({
                .record_index = fill_style_object.record_index,
                .object_name = fill_style_object.object_name,
                .unique_id = fill_style_object.unique_id
            });
        }

        const auto fill_style_result = copperfin::vfp::set_visual_object_fill_style({
            .path = parse_result.request.path,
            .objects = fill_style_objects,
            .fill_style = parse_result.request.fill_style
        });

        if (!fill_style_result.ok) {
            std::cout << "status: error\n";
            std::cout << studio_error_prefix() << fill_style_result.error << "\n";
            return 4;
        }
    }
    if (parse_result.request.scale_mode_object) {
        std::vector<copperfin::vfp::VisualObjectAlignmentTarget> scale_mode_objects;
        scale_mode_objects.reserve(parse_result.request.scale_mode_objects.size());
        for (const auto& scale_mode_object : parse_result.request.scale_mode_objects) {
            scale_mode_objects.push_back({
                .record_index = scale_mode_object.record_index,
                .object_name = scale_mode_object.object_name,
                .unique_id = scale_mode_object.unique_id
            });
        }

        const auto scale_mode_result = copperfin::vfp::set_visual_object_scale_mode({
            .path = parse_result.request.path,
            .objects = scale_mode_objects,
            .scale_mode = parse_result.request.scale_mode
        });

        if (!scale_mode_result.ok) {
            std::cout << "status: error\n";
            std::cout << studio_error_prefix() << scale_mode_result.error << "\n";
            return 4;
        }
    }
    if (parse_result.request.buffer_mode_object) {
        std::vector<copperfin::vfp::VisualObjectAlignmentTarget> buffer_mode_objects;
        buffer_mode_objects.reserve(parse_result.request.buffer_mode_objects.size());
        for (const auto& buffer_mode_object : parse_result.request.buffer_mode_objects) {
            buffer_mode_objects.push_back({
                .record_index = buffer_mode_object.record_index,
                .object_name = buffer_mode_object.object_name,
                .unique_id = buffer_mode_object.unique_id
            });
        }

        const auto buffer_mode_result = copperfin::vfp::set_visual_object_buffer_mode({
            .path = parse_result.request.path,
            .objects = buffer_mode_objects,
            .buffer_mode = parse_result.request.buffer_mode
        });

        if (!buffer_mode_result.ok) {
            std::cout << "status: error\n";
            std::cout << studio_error_prefix() << buffer_mode_result.error << "\n";
            return 4;
        }
    }
    if (parse_result.request.buffer_mode_override_object) {
        std::vector<copperfin::vfp::VisualObjectAlignmentTarget> buffer_mode_override_objects;
        buffer_mode_override_objects.reserve(parse_result.request.buffer_mode_override_objects.size());
        for (const auto& buffer_mode_override_object : parse_result.request.buffer_mode_override_objects) {
            buffer_mode_override_objects.push_back({
                .record_index = buffer_mode_override_object.record_index,
                .object_name = buffer_mode_override_object.object_name,
                .unique_id = buffer_mode_override_object.unique_id
            });
        }

        const auto buffer_mode_override_result = copperfin::vfp::set_visual_object_buffer_mode_override({
            .path = parse_result.request.path,
            .objects = buffer_mode_override_objects,
            .buffer_mode_override = parse_result.request.buffer_mode_override
        });

        if (!buffer_mode_override_result.ok) {
            std::cout << "status: error\n";
            std::cout << studio_error_prefix() << buffer_mode_override_result.error << "\n";
            return 4;
        }
    }
    if (parse_result.request.data_session_object) {
        std::vector<copperfin::vfp::VisualObjectAlignmentTarget> data_session_objects;
        data_session_objects.reserve(parse_result.request.data_session_objects.size());
        for (const auto& data_session_object : parse_result.request.data_session_objects) {
            data_session_objects.push_back({
                .record_index = data_session_object.record_index,
                .object_name = data_session_object.object_name,
                .unique_id = data_session_object.unique_id
            });
        }

        const auto data_session_result = copperfin::vfp::set_visual_object_data_session({
            .path = parse_result.request.path,
            .objects = data_session_objects,
            .data_session = parse_result.request.data_session
        });

        if (!data_session_result.ok) {
            std::cout << "status: error\n";
            std::cout << studio_error_prefix() << data_session_result.error << "\n";
            return 4;
        }
    }
    if (parse_result.request.grid_line_color_object) {
        std::vector<copperfin::vfp::VisualObjectAlignmentTarget> grid_line_color_objects;
        grid_line_color_objects.reserve(parse_result.request.grid_line_color_objects.size());
        for (const auto& grid_line_color_object : parse_result.request.grid_line_color_objects) {
            grid_line_color_objects.push_back({
                .record_index = grid_line_color_object.record_index,
                .object_name = grid_line_color_object.object_name,
                .unique_id = grid_line_color_object.unique_id
            });
        }

        const auto grid_line_color_result = copperfin::vfp::set_visual_object_grid_line_color({
            .path = parse_result.request.path,
            .objects = grid_line_color_objects,
            .grid_line_color = parse_result.request.grid_line_color
        });

        if (!grid_line_color_result.ok) {
            std::cout << "status: error\n";
            std::cout << studio_error_prefix() << grid_line_color_result.error << "\n";
            return 4;
        }
    }
    if (parse_result.request.header_height_object) {
        std::vector<copperfin::vfp::VisualObjectAlignmentTarget> header_height_objects;
        header_height_objects.reserve(parse_result.request.header_height_objects.size());
        for (const auto& header_height_object : parse_result.request.header_height_objects) {
            header_height_objects.push_back({
                .record_index = header_height_object.record_index,
                .object_name = header_height_object.object_name,
                .unique_id = header_height_object.unique_id
            });
        }

        const auto header_height_result = copperfin::vfp::set_visual_object_header_height({
            .path = parse_result.request.path,
            .objects = header_height_objects,
            .header_height = parse_result.request.header_height
        });

        if (!header_height_result.ok) {
            std::cout << "status: error\n";
            std::cout << studio_error_prefix() << header_height_result.error << "\n";
            return 4;
        }
    }
    if (parse_result.request.row_height_object) {
        std::vector<copperfin::vfp::VisualObjectAlignmentTarget> row_height_objects;
        row_height_objects.reserve(parse_result.request.row_height_objects.size());
        for (const auto& row_height_object : parse_result.request.row_height_objects) {
            row_height_objects.push_back({
                .record_index = row_height_object.record_index,
                .object_name = row_height_object.object_name,
                .unique_id = row_height_object.unique_id
            });
        }

        const auto row_height_result = copperfin::vfp::set_visual_object_row_height({
            .path = parse_result.request.path,
            .objects = row_height_objects,
            .row_height = parse_result.request.row_height
        });

        if (!row_height_result.ok) {
            std::cout << "status: error\n";
            std::cout << studio_error_prefix() << row_height_result.error << "\n";
            return 4;
        }
    }
    if (parse_result.request.lock_columns_object) {
        std::vector<copperfin::vfp::VisualObjectAlignmentTarget> lock_columns_objects;
        lock_columns_objects.reserve(parse_result.request.lock_columns_objects.size());
        for (const auto& lock_columns_object : parse_result.request.lock_columns_objects) {
            lock_columns_objects.push_back({
                .record_index = lock_columns_object.record_index,
                .object_name = lock_columns_object.object_name,
                .unique_id = lock_columns_object.unique_id
            });
        }

        const auto lock_columns_result = copperfin::vfp::set_visual_object_lock_columns({
            .path = parse_result.request.path,
            .objects = lock_columns_objects,
            .lock_columns = parse_result.request.lock_columns
        });

        if (!lock_columns_result.ok) {
            std::cout << "status: error\n";
            std::cout << studio_error_prefix() << lock_columns_result.error << "\n";
            return 4;
        }
    }
    if (parse_result.request.lock_columns_left_object) {
        std::vector<copperfin::vfp::VisualObjectAlignmentTarget> lock_columns_left_objects;
        lock_columns_left_objects.reserve(parse_result.request.lock_columns_left_objects.size());
        for (const auto& lock_columns_left_object : parse_result.request.lock_columns_left_objects) {
            lock_columns_left_objects.push_back({
                .record_index = lock_columns_left_object.record_index,
                .object_name = lock_columns_left_object.object_name,
                .unique_id = lock_columns_left_object.unique_id
            });
        }

        const auto lock_columns_left_result = copperfin::vfp::set_visual_object_lock_columns_left({
            .path = parse_result.request.path,
            .objects = lock_columns_left_objects,
            .lock_columns_left = parse_result.request.lock_columns_left
        });

        if (!lock_columns_left_result.ok) {
            std::cout << "status: error\n";
            std::cout << studio_error_prefix() << lock_columns_left_result.error << "\n";
            return 4;
        }
    }
    if (parse_result.request.grid_line_width_object) {
        std::vector<copperfin::vfp::VisualObjectAlignmentTarget> grid_line_width_objects;
        grid_line_width_objects.reserve(parse_result.request.grid_line_width_objects.size());
        for (const auto& grid_line_width_object : parse_result.request.grid_line_width_objects) {
            grid_line_width_objects.push_back({
                .record_index = grid_line_width_object.record_index,
                .object_name = grid_line_width_object.object_name,
                .unique_id = grid_line_width_object.unique_id
            });
        }

        const auto grid_line_width_result = copperfin::vfp::set_visual_object_grid_line_width({
            .path = parse_result.request.path,
            .objects = grid_line_width_objects,
            .grid_line_width = parse_result.request.grid_line_width
        });

        if (!grid_line_width_result.ok) {
            std::cout << "status: error\n";
            std::cout << studio_error_prefix() << grid_line_width_result.error << "\n";
            return 4;
        }
    }
    if (parse_result.request.grid_lines_object) {
        std::vector<copperfin::vfp::VisualObjectAlignmentTarget> grid_lines_objects;
        grid_lines_objects.reserve(parse_result.request.grid_lines_objects.size());
        for (const auto& grid_lines_object : parse_result.request.grid_lines_objects) {
            grid_lines_objects.push_back({
                .record_index = grid_lines_object.record_index,
                .object_name = grid_lines_object.object_name,
                .unique_id = grid_lines_object.unique_id
            });
        }

        const auto grid_lines_result = copperfin::vfp::set_visual_object_grid_lines({
            .path = parse_result.request.path,
            .objects = grid_lines_objects,
            .grid_lines = parse_result.request.grid_lines
        });

        if (!grid_lines_result.ok) {
            std::cout << "status: error\n";
            std::cout << studio_error_prefix() << grid_lines_result.error << "\n";
            return 4;
        }
    }
    if (parse_result.request.highlight_row_line_width_object) {
        std::vector<copperfin::vfp::VisualObjectAlignmentTarget> highlight_row_line_width_objects;
        highlight_row_line_width_objects.reserve(parse_result.request.highlight_row_line_width_objects.size());
        for (const auto& highlight_row_line_width_object : parse_result.request.highlight_row_line_width_objects) {
            highlight_row_line_width_objects.push_back({
                .record_index = highlight_row_line_width_object.record_index,
                .object_name = highlight_row_line_width_object.object_name,
                .unique_id = highlight_row_line_width_object.unique_id
            });
        }

        const auto highlight_row_line_width_result = copperfin::vfp::set_visual_object_highlight_row_line_width({
            .path = parse_result.request.path,
            .objects = highlight_row_line_width_objects,
            .highlight_row_line_width = parse_result.request.highlight_row_line_width
        });

        if (!highlight_row_line_width_result.ok) {
            std::cout << "status: error\n";
            std::cout << studio_error_prefix() << highlight_row_line_width_result.error << "\n";
            return 4;
        }
    }
    if (parse_result.request.partition_object) {
        std::vector<copperfin::vfp::VisualObjectAlignmentTarget> partition_objects;
        partition_objects.reserve(parse_result.request.partition_objects.size());
        for (const auto& partition_object : parse_result.request.partition_objects) {
            partition_objects.push_back({
                .record_index = partition_object.record_index,
                .object_name = partition_object.object_name,
                .unique_id = partition_object.unique_id
            });
        }

        const auto partition_result = copperfin::vfp::set_visual_object_partition({
            .path = parse_result.request.path,
            .objects = partition_objects,
            .partition = parse_result.request.partition
        });

        if (!partition_result.ok) {
            std::cout << "status: error\n";
            std::cout << studio_error_prefix() << partition_result.error << "\n";
            return 4;
        }
    }
    if (parse_result.request.record_source_type_object) {
        std::vector<copperfin::vfp::VisualObjectAlignmentTarget> record_source_type_objects;
        record_source_type_objects.reserve(parse_result.request.record_source_type_objects.size());
        for (const auto& record_source_type_object : parse_result.request.record_source_type_objects) {
            record_source_type_objects.push_back({
                .record_index = record_source_type_object.record_index,
                .object_name = record_source_type_object.object_name,
                .unique_id = record_source_type_object.unique_id
            });
        }

        const auto record_source_type_result = copperfin::vfp::set_visual_object_record_source_type({
            .path = parse_result.request.path,
            .objects = record_source_type_objects,
            .record_source_type = parse_result.request.record_source_type
        });

        if (!record_source_type_result.ok) {
            std::cout << "status: error\n";
            std::cout << studio_error_prefix() << record_source_type_result.error << "\n";
            return 4;
        }
    }
    if (parse_result.request.column_order_object) {
        std::vector<copperfin::vfp::VisualObjectAlignmentTarget> column_order_objects;
        column_order_objects.reserve(parse_result.request.column_order_objects.size());
        for (const auto& column_order_object : parse_result.request.column_order_objects) {
            column_order_objects.push_back({
                .record_index = column_order_object.record_index,
                .object_name = column_order_object.object_name,
                .unique_id = column_order_object.unique_id
            });
        }

        const auto column_order_result = copperfin::vfp::set_visual_object_column_order({
            .path = parse_result.request.path,
            .objects = column_order_objects,
            .column_order = parse_result.request.column_order
        });

        if (!column_order_result.ok) {
            std::cout << "status: error\n";
            std::cout << studio_error_prefix() << column_order_result.error << "\n";
            return 4;
        }
    }
    if (parse_result.request.highlight_style_object) {
        std::vector<copperfin::vfp::VisualObjectAlignmentTarget> highlight_style_objects;
        highlight_style_objects.reserve(parse_result.request.highlight_style_objects.size());
        for (const auto& highlight_style_object : parse_result.request.highlight_style_objects) {
            highlight_style_objects.push_back({
                .record_index = highlight_style_object.record_index,
                .object_name = highlight_style_object.object_name,
                .unique_id = highlight_style_object.unique_id
            });
        }

        const auto highlight_style_result = copperfin::vfp::set_visual_object_highlight_style({
            .path = parse_result.request.path,
            .objects = highlight_style_objects,
            .highlight_style = parse_result.request.highlight_style
        });

        if (!highlight_style_result.ok) {
            std::cout << "status: error\n";
            std::cout << studio_error_prefix() << highlight_style_result.error << "\n";
            return 4;
        }
    }
    if (parse_result.request.child_order_object) {
        std::vector<copperfin::vfp::VisualObjectAlignmentTarget> child_order_objects;
        child_order_objects.reserve(parse_result.request.child_order_objects.size());
        for (const auto& child_order_object : parse_result.request.child_order_objects) {
            child_order_objects.push_back({
                .record_index = child_order_object.record_index,
                .object_name = child_order_object.object_name,
                .unique_id = child_order_object.unique_id
            });
        }

        const auto child_order_result = copperfin::vfp::set_visual_object_child_order({
            .path = parse_result.request.path,
            .objects = child_order_objects,
            .child_order = parse_result.request.child_order
        });

        if (!child_order_result.ok) {
            std::cout << "status: error\n";
            std::cout << studio_error_prefix() << child_order_result.error << "\n";
            return 4;
        }
    }
    if (parse_result.request.fill_color_object) {
        std::vector<copperfin::vfp::VisualObjectAlignmentTarget> fill_color_objects;
        fill_color_objects.reserve(parse_result.request.fill_color_objects.size());
        for (const auto& fill_color_object : parse_result.request.fill_color_objects) {
            fill_color_objects.push_back({
                .record_index = fill_color_object.record_index,
                .object_name = fill_color_object.object_name,
                .unique_id = fill_color_object.unique_id
            });
        }

        const auto fill_color_result = copperfin::vfp::set_visual_object_fill_color({
            .path = parse_result.request.path,
            .objects = fill_color_objects,
            .fill_color = parse_result.request.fill_color
        });

        if (!fill_color_result.ok) {
            std::cout << "status: error\n";
            std::cout << studio_error_prefix() << fill_color_result.error << "\n";
            return 4;
        }
    }
    if (parse_result.request.list_item_id_object) {
        std::vector<copperfin::vfp::VisualObjectAlignmentTarget> list_item_id_objects;
        list_item_id_objects.reserve(parse_result.request.list_item_id_objects.size());
        for (const auto& list_item_id_object : parse_result.request.list_item_id_objects) {
            list_item_id_objects.push_back({
                .record_index = list_item_id_object.record_index,
                .object_name = list_item_id_object.object_name,
                .unique_id = list_item_id_object.unique_id
            });
        }

        const auto list_item_id_result = copperfin::vfp::set_visual_object_list_item_id({
            .path = parse_result.request.path,
            .objects = list_item_id_objects,
            .list_item_id = parse_result.request.list_item_id
        });

        if (!list_item_id_result.ok) {
            std::cout << "status: error\n";
            std::cout << studio_error_prefix() << list_item_id_result.error << "\n";
            return 4;
        }
    }
    if (parse_result.request.tab_orientation_object) {
        std::vector<copperfin::vfp::VisualObjectAlignmentTarget> tab_orientation_objects;
        tab_orientation_objects.reserve(parse_result.request.tab_orientation_objects.size());
        for (const auto& tab_orientation_object : parse_result.request.tab_orientation_objects) {
            tab_orientation_objects.push_back({
                .record_index = tab_orientation_object.record_index,
                .object_name = tab_orientation_object.object_name,
                .unique_id = tab_orientation_object.unique_id
            });
        }

        const auto tab_orientation_result = copperfin::vfp::set_visual_object_tab_orientation({
            .path = parse_result.request.path,
            .objects = tab_orientation_objects,
            .tab_orientation = parse_result.request.tab_orientation
        });

        if (!tab_orientation_result.ok) {
            std::cout << "status: error\n";
            std::cout << studio_error_prefix() << tab_orientation_result.error << "\n";
            return 4;
        }
    }
    if (parse_result.request.display_orientation_object) {
        std::vector<copperfin::vfp::VisualObjectAlignmentTarget> display_orientation_objects;
        display_orientation_objects.reserve(parse_result.request.display_orientation_objects.size());
        for (const auto& display_orientation_object : parse_result.request.display_orientation_objects) {
            display_orientation_objects.push_back({
                .record_index = display_orientation_object.record_index,
                .object_name = display_orientation_object.object_name,
                .unique_id = display_orientation_object.unique_id
            });
        }

        const auto display_orientation_result = copperfin::vfp::set_visual_object_display_orientation({
            .path = parse_result.request.path,
            .objects = display_orientation_objects,
            .display_orientation = parse_result.request.display_orientation
        });

        if (!display_orientation_result.ok) {
            std::cout << "status: error\n";
            std::cout << studio_error_prefix() << display_orientation_result.error << "\n";
            return 4;
        }
    }
    if (parse_result.request.help_context_id_object) {
        std::vector<copperfin::vfp::VisualObjectAlignmentTarget> help_context_id_objects;
        help_context_id_objects.reserve(parse_result.request.help_context_id_objects.size());
        for (const auto& help_context_id_object : parse_result.request.help_context_id_objects) {
            help_context_id_objects.push_back({
                .record_index = help_context_id_object.record_index,
                .object_name = help_context_id_object.object_name,
                .unique_id = help_context_id_object.unique_id
            });
        }

        const auto help_context_id_result = copperfin::vfp::set_visual_object_help_context_id({
            .path = parse_result.request.path,
            .objects = help_context_id_objects,
            .help_context_id = parse_result.request.help_context_id
        });

        if (!help_context_id_result.ok) {
            std::cout << "status: error\n";
            std::cout << studio_error_prefix() << help_context_id_result.error << "\n";
            return 4;
        }
    }
    if (parse_result.request.whats_this_help_id_object) {
        std::vector<copperfin::vfp::VisualObjectAlignmentTarget> whats_this_help_id_objects;
        whats_this_help_id_objects.reserve(parse_result.request.whats_this_help_id_objects.size());
        for (const auto& whats_this_help_id_object : parse_result.request.whats_this_help_id_objects) {
            whats_this_help_id_objects.push_back({
                .record_index = whats_this_help_id_object.record_index,
                .object_name = whats_this_help_id_object.object_name,
                .unique_id = whats_this_help_id_object.unique_id
            });
        }

        const auto whats_this_help_id_result = copperfin::vfp::set_visual_object_whats_this_help_id({
            .path = parse_result.request.path,
            .objects = whats_this_help_id_objects,
            .whats_this_help_id = parse_result.request.whats_this_help_id
        });

        if (!whats_this_help_id_result.ok) {
            std::cout << "status: error\n";
            std::cout << studio_error_prefix() << whats_this_help_id_result.error << "\n";
            return 4;
        }
    }
    if (parse_result.request.whats_this_help_object) {
        std::vector<copperfin::vfp::VisualObjectAlignmentTarget> whats_this_help_objects;
        whats_this_help_objects.reserve(parse_result.request.whats_this_help_objects.size());
        for (const auto& whats_this_help_object : parse_result.request.whats_this_help_objects) {
            whats_this_help_objects.push_back({
                .record_index = whats_this_help_object.record_index,
                .object_name = whats_this_help_object.object_name,
                .unique_id = whats_this_help_object.unique_id
            });
        }

        const auto whats_this_help_result = copperfin::vfp::set_visual_object_whats_this_help({
            .path = parse_result.request.path,
            .objects = whats_this_help_objects,
            .whats_this_help = parse_result.request.whats_this_help
        });

        if (!whats_this_help_result.ok) {
            std::cout << "status: error\n";
            std::cout << studio_error_prefix() << whats_this_help_result.error << "\n";
            return 4;
        }
    }
    if (parse_result.request.whats_this_button_object) {
        std::vector<copperfin::vfp::VisualObjectAlignmentTarget> whats_this_button_objects;
        whats_this_button_objects.reserve(parse_result.request.whats_this_button_objects.size());
        for (const auto& whats_this_button_object : parse_result.request.whats_this_button_objects) {
            whats_this_button_objects.push_back({
                .record_index = whats_this_button_object.record_index,
                .object_name = whats_this_button_object.object_name,
                .unique_id = whats_this_button_object.unique_id
            });
        }

        const auto whats_this_button_result = copperfin::vfp::set_visual_object_whats_this_button({
            .path = parse_result.request.path,
            .objects = whats_this_button_objects,
            .whats_this_button = parse_result.request.whats_this_button
        });

        if (!whats_this_button_result.ok) {
            std::cout << "status: error\n";
            std::cout << studio_error_prefix() << whats_this_button_result.error << "\n";
            return 4;
        }
    }
    if (parse_result.request.record_source_object) {
        std::vector<copperfin::vfp::VisualObjectAlignmentTarget> record_source_objects;
        record_source_objects.reserve(parse_result.request.record_source_objects.size());
        for (const auto& record_source_object : parse_result.request.record_source_objects) {
            record_source_objects.push_back({
                .record_index = record_source_object.record_index,
                .object_name = record_source_object.object_name,
                .unique_id = record_source_object.unique_id
            });
        }

        const auto record_source_result = copperfin::vfp::set_visual_object_record_source({
            .path = parse_result.request.path,
            .objects = record_source_objects,
            .record_source = parse_result.request.record_source
        });

        if (!record_source_result.ok) {
            std::cout << "status: error\n";
            std::cout << studio_error_prefix() << record_source_result.error << "\n";
            return 4;
        }
    }
    if (parse_result.request.form_set_class_object) {
        std::vector<copperfin::vfp::VisualObjectAlignmentTarget> form_set_class_objects;
        form_set_class_objects.reserve(parse_result.request.form_set_class_objects.size());
        for (const auto& form_set_class_object : parse_result.request.form_set_class_objects) {
            form_set_class_objects.push_back({
                .record_index = form_set_class_object.record_index,
                .object_name = form_set_class_object.object_name,
                .unique_id = form_set_class_object.unique_id
            });
        }

        const auto form_set_class_result = copperfin::vfp::set_visual_object_form_set_class({
            .path = parse_result.request.path,
            .objects = form_set_class_objects,
            .form_set_class = parse_result.request.form_set_class
        });

        if (!form_set_class_result.ok) {
            std::cout << "status: error\n";
            std::cout << studio_error_prefix() << form_set_class_result.error << "\n";
            return 4;
        }
    }
    if (parse_result.request.default_file_path_object) {
        std::vector<copperfin::vfp::VisualObjectAlignmentTarget> default_file_path_objects;
        default_file_path_objects.reserve(parse_result.request.default_file_path_objects.size());
        for (const auto& default_file_path_object : parse_result.request.default_file_path_objects) {
            default_file_path_objects.push_back({
                .record_index = default_file_path_object.record_index,
                .object_name = default_file_path_object.object_name,
                .unique_id = default_file_path_object.unique_id
            });
        }

        const auto default_file_path_result = copperfin::vfp::set_visual_object_default_file_path({
            .path = parse_result.request.path,
            .objects = default_file_path_objects,
            .default_file_path = parse_result.request.default_file_path
        });

        if (!default_file_path_result.ok) {
            std::cout << "status: error\n";
            std::cout << studio_error_prefix() << default_file_path_result.error << "\n";
            return 4;
        }
    }
    if (parse_result.request.initial_selected_alias_object) {
        std::vector<copperfin::vfp::VisualObjectAlignmentTarget> initial_selected_alias_objects;
        initial_selected_alias_objects.reserve(parse_result.request.initial_selected_alias_objects.size());
        for (const auto& initial_selected_alias_object : parse_result.request.initial_selected_alias_objects) {
            initial_selected_alias_objects.push_back({
                .record_index = initial_selected_alias_object.record_index,
                .object_name = initial_selected_alias_object.object_name,
                .unique_id = initial_selected_alias_object.unique_id
            });
        }

        const auto initial_selected_alias_result = copperfin::vfp::set_visual_object_initial_selected_alias({
            .path = parse_result.request.path,
            .objects = initial_selected_alias_objects,
            .initial_selected_alias = parse_result.request.initial_selected_alias
        });

        if (!initial_selected_alias_result.ok) {
            std::cout << "status: error\n";
            std::cout << studio_error_prefix() << initial_selected_alias_result.error << "\n";
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
            std::cout << studio_error_prefix() << tooltip_text_result.error << "\n";
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
            std::cout << studio_error_prefix() << status_bar_text_result.error << "\n";
            return 4;
        }
    }
    if (parse_result.request.link_master_object) {
        std::vector<copperfin::vfp::VisualObjectAlignmentTarget> link_master_objects;
        link_master_objects.reserve(parse_result.request.link_master_objects.size());
        for (const auto& link_master_object : parse_result.request.link_master_objects) {
            link_master_objects.push_back({
                .record_index = link_master_object.record_index,
                .object_name = link_master_object.object_name,
                .unique_id = link_master_object.unique_id
            });
        }

        const auto link_master_result = copperfin::vfp::set_visual_object_link_master({
            .path = parse_result.request.path,
            .objects = link_master_objects,
            .link_master = parse_result.request.link_master
        });

        if (!link_master_result.ok) {
            std::cout << "status: error\n";
            std::cout << studio_error_prefix() << link_master_result.error << "\n";
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
            std::cout << studio_error_prefix() << control_source_result.error << "\n";
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
            std::cout << studio_error_prefix() << current_control_result.error << "\n";
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
            std::cout << studio_error_prefix() << input_mask_result.error << "\n";
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
            std::cout << studio_error_prefix() << format_result.error << "\n";
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
            std::cout << studio_error_prefix() << row_source_result.error << "\n";
            return 4;
        }
    }
    if (parse_result.request.column_widths_object) {
        std::vector<copperfin::vfp::VisualObjectAlignmentTarget> column_widths_objects;
        column_widths_objects.reserve(parse_result.request.column_widths_objects.size());
        for (const auto& column_widths_object : parse_result.request.column_widths_objects) {
            column_widths_objects.push_back({
                .record_index = column_widths_object.record_index,
                .object_name = column_widths_object.object_name,
                .unique_id = column_widths_object.unique_id
            });
        }

        const auto column_widths_result = copperfin::vfp::set_visual_object_column_widths({
            .path = parse_result.request.path,
            .objects = column_widths_objects,
            .column_widths = parse_result.request.column_widths
        });

        if (!column_widths_result.ok) {
            std::cout << "status: error\n";
            std::cout << studio_error_prefix() << column_widths_result.error << "\n";
            return 4;
        }
    }
    if (parse_result.request.column_lines_object) {
        std::vector<copperfin::vfp::VisualObjectAlignmentTarget> column_lines_objects;
        column_lines_objects.reserve(parse_result.request.column_lines_objects.size());
        for (const auto& column_lines_object : parse_result.request.column_lines_objects) {
            column_lines_objects.push_back({
                .record_index = column_lines_object.record_index,
                .object_name = column_lines_object.object_name,
                .unique_id = column_lines_object.unique_id
            });
        }

        const auto column_lines_result = copperfin::vfp::set_visual_object_column_lines({
            .path = parse_result.request.path,
            .objects = column_lines_objects,
            .column_lines = parse_result.request.column_lines
        });

        if (!column_lines_result.ok) {
            std::cout << "status: error\n";
            std::cout << studio_error_prefix() << column_lines_result.error << "\n";
            return 4;
        }
    }
    if (parse_result.request.integral_height_object) {
        std::vector<copperfin::vfp::VisualObjectAlignmentTarget> integral_height_objects;
        integral_height_objects.reserve(parse_result.request.integral_height_objects.size());
        for (const auto& integral_height_object : parse_result.request.integral_height_objects) {
            integral_height_objects.push_back({
                .record_index = integral_height_object.record_index,
                .object_name = integral_height_object.object_name,
                .unique_id = integral_height_object.unique_id
            });
        }

        const auto integral_height_result = copperfin::vfp::set_visual_object_integral_height({
            .path = parse_result.request.path,
            .objects = integral_height_objects,
            .integral_height = parse_result.request.integral_height
        });

        if (!integral_height_result.ok) {
            std::cout << "status: error\n";
            std::cout << studio_error_prefix() << integral_height_result.error << "\n";
            return 4;
        }
    }
    if (parse_result.request.incremental_search_object) {
        std::vector<copperfin::vfp::VisualObjectAlignmentTarget> incremental_search_objects;
        incremental_search_objects.reserve(parse_result.request.incremental_search_objects.size());
        for (const auto& incremental_search_object : parse_result.request.incremental_search_objects) {
            incremental_search_objects.push_back({
                .record_index = incremental_search_object.record_index,
                .object_name = incremental_search_object.object_name,
                .unique_id = incremental_search_object.unique_id
            });
        }

        const auto incremental_search_result = copperfin::vfp::set_visual_object_incremental_search({
            .path = parse_result.request.path,
            .objects = incremental_search_objects,
            .incremental_search = parse_result.request.incremental_search
        });

        if (!incremental_search_result.ok) {
            std::cout << "status: error\n";
            std::cout << studio_error_prefix() << incremental_search_result.error << "\n";
            return 4;
        }
    }
    if (parse_result.request.multi_select_object) {
        std::vector<copperfin::vfp::VisualObjectAlignmentTarget> multi_select_objects;
        multi_select_objects.reserve(parse_result.request.multi_select_objects.size());
        for (const auto& multi_select_object : parse_result.request.multi_select_objects) {
            multi_select_objects.push_back({
                .record_index = multi_select_object.record_index,
                .object_name = multi_select_object.object_name,
                .unique_id = multi_select_object.unique_id
            });
        }

        const auto multi_select_result = copperfin::vfp::set_visual_object_multi_select({
            .path = parse_result.request.path,
            .objects = multi_select_objects,
            .multi_select = parse_result.request.multi_select
        });

        if (!multi_select_result.ok) {
            std::cout << "status: error\n";
            std::cout << studio_error_prefix() << multi_select_result.error << "\n";
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
            std::cout << studio_error_prefix() << row_source_type_result.error << "\n";
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
            std::cout << studio_error_prefix() << bound_column_result.error << "\n";
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
            std::cout << studio_error_prefix() << column_count_result.error << "\n";
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
            std::cout << studio_error_prefix() << style_result.error << "\n";
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
            std::cout << studio_error_prefix() << list_index_result.error << "\n";
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
            std::cout << studio_error_prefix() << left_column_result.error << "\n";
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
            std::cout << studio_error_prefix() << display_value_result.error << "\n";
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
            std::cout << studio_error_prefix() << selected_back_color_result.error << "\n";
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
            std::cout << studio_error_prefix() << selected_fore_color_result.error << "\n";
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
            std::cout << studio_error_prefix() << selected_item_back_color_result.error << "\n";
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
            std::cout << studio_error_prefix() << selected_item_fore_color_result.error << "\n";
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
            std::cout << studio_error_prefix() << disabled_item_back_color_result.error << "\n";
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
            std::cout << studio_error_prefix() << disabled_item_fore_color_result.error << "\n";
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
            std::cout << studio_error_prefix() << item_back_color_result.error << "\n";
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
            std::cout << studio_error_prefix() << item_fore_color_result.error << "\n";
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
            std::cout << studio_error_prefix() << highlight_back_color_result.error << "\n";
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
            std::cout << studio_error_prefix() << highlight_fore_color_result.error << "\n";
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
            std::cout << studio_error_prefix() << back_color_result.error << "\n";
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
            std::cout << studio_error_prefix() << fore_color_result.error << "\n";
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
            std::cout << studio_error_prefix() << disabled_back_color_result.error << "\n";
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
            std::cout << studio_error_prefix() << disabled_fore_color_result.error << "\n";
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
            std::cout << studio_error_prefix() << dynamic_back_color_result.error << "\n";
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
            std::cout << studio_error_prefix() << dynamic_fore_color_result.error << "\n";
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
            std::cout << studio_error_prefix() << closable_result.error << "\n";
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
            std::cout << studio_error_prefix() << control_box_result.error << "\n";
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
            std::cout << studio_error_prefix() << allow_output_result.error << "\n";
            return 4;
        }
    }
    if (parse_result.request.bind_controls_object) {
        std::vector<copperfin::vfp::VisualObjectAlignmentTarget> bind_controls_objects;
        bind_controls_objects.reserve(parse_result.request.bind_controls_objects.size());
        for (const auto& bind_controls_object : parse_result.request.bind_controls_objects) {
            bind_controls_objects.push_back({
                .record_index = bind_controls_object.record_index,
                .object_name = bind_controls_object.object_name,
                .unique_id = bind_controls_object.unique_id
            });
        }

        const auto bind_controls_result = copperfin::vfp::set_visual_object_bind_controls({
            .path = parse_result.request.path,
            .objects = bind_controls_objects,
            .bind_controls = parse_result.request.bind_controls
        });

        if (!bind_controls_result.ok) {
            std::cout << "status: error\n";
            std::cout << studio_error_prefix() << bind_controls_result.error << "\n";
            return 4;
        }
    }
    if (parse_result.request.auto_verb_menu_object) {
        std::vector<copperfin::vfp::VisualObjectAlignmentTarget> auto_verb_menu_objects;
        auto_verb_menu_objects.reserve(parse_result.request.auto_verb_menu_objects.size());
        for (const auto& auto_verb_menu_object : parse_result.request.auto_verb_menu_objects) {
            auto_verb_menu_objects.push_back({
                .record_index = auto_verb_menu_object.record_index,
                .object_name = auto_verb_menu_object.object_name,
                .unique_id = auto_verb_menu_object.unique_id
            });
        }

        const auto auto_verb_menu_result = copperfin::vfp::set_visual_object_auto_verb_menu({
            .path = parse_result.request.path,
            .objects = auto_verb_menu_objects,
            .auto_verb_menu = parse_result.request.auto_verb_menu
        });

        if (!auto_verb_menu_result.ok) {
            std::cout << "status: error\n";
            std::cout << studio_error_prefix() << auto_verb_menu_result.error << "\n";
            return 4;
        }
    }
    if (parse_result.request.desktop_object) {
        std::vector<copperfin::vfp::VisualObjectAlignmentTarget> desktop_objects;
        desktop_objects.reserve(parse_result.request.desktop_objects.size());
        for (const auto& desktop_object : parse_result.request.desktop_objects) {
            desktop_objects.push_back({
                .record_index = desktop_object.record_index,
                .object_name = desktop_object.object_name,
                .unique_id = desktop_object.unique_id
            });
        }

        const auto desktop_result = copperfin::vfp::set_visual_object_desktop({
            .path = parse_result.request.path,
            .objects = desktop_objects,
            .desktop = parse_result.request.desktop
        });

        if (!desktop_result.ok) {
            std::cout << "status: error\n";
            std::cout << studio_error_prefix() << desktop_result.error << "\n";
            return 4;
        }
    }
    if (parse_result.request.key_preview_object) {
        std::vector<copperfin::vfp::VisualObjectAlignmentTarget> key_preview_objects;
        key_preview_objects.reserve(parse_result.request.key_preview_objects.size());
        for (const auto& key_preview_object : parse_result.request.key_preview_objects) {
            key_preview_objects.push_back({
                .record_index = key_preview_object.record_index,
                .object_name = key_preview_object.object_name,
                .unique_id = key_preview_object.unique_id
            });
        }

        const auto key_preview_result = copperfin::vfp::set_visual_object_key_preview({
            .path = parse_result.request.path,
            .objects = key_preview_objects,
            .key_preview = parse_result.request.key_preview
        });

        if (!key_preview_result.ok) {
            std::cout << "status: error\n";
            std::cout << studio_error_prefix() << key_preview_result.error << "\n";
            return 4;
        }
    }
    if (parse_result.request.mac_desktop_object) {
        std::vector<copperfin::vfp::VisualObjectAlignmentTarget> mac_desktop_objects;
        mac_desktop_objects.reserve(parse_result.request.mac_desktop_objects.size());
        for (const auto& mac_desktop_object : parse_result.request.mac_desktop_objects) {
            mac_desktop_objects.push_back({
                .record_index = mac_desktop_object.record_index,
                .object_name = mac_desktop_object.object_name,
                .unique_id = mac_desktop_object.unique_id
            });
        }

        const auto mac_desktop_result = copperfin::vfp::set_visual_object_mac_desktop({
            .path = parse_result.request.path,
            .objects = mac_desktop_objects,
            .mac_desktop = parse_result.request.mac_desktop
        });

        if (!mac_desktop_result.ok) {
            std::cout << "status: error\n";
            std::cout << studio_error_prefix() << mac_desktop_result.error << "\n";
            return 4;
        }
    }
    if (parse_result.request.max_button_object) {
        std::vector<copperfin::vfp::VisualObjectAlignmentTarget> max_button_objects;
        max_button_objects.reserve(parse_result.request.max_button_objects.size());
        for (const auto& max_button_object : parse_result.request.max_button_objects) {
            max_button_objects.push_back({
                .record_index = max_button_object.record_index,
                .object_name = max_button_object.object_name,
                .unique_id = max_button_object.unique_id
            });
        }

        const auto max_button_result = copperfin::vfp::set_visual_object_max_button({
            .path = parse_result.request.path,
            .objects = max_button_objects,
            .max_button = parse_result.request.max_button
        });

        if (!max_button_result.ok) {
            std::cout << "status: error\n";
            std::cout << studio_error_prefix() << max_button_result.error << "\n";
            return 4;
        }
    }
    if (parse_result.request.min_button_object) {
        std::vector<copperfin::vfp::VisualObjectAlignmentTarget> min_button_objects;
        min_button_objects.reserve(parse_result.request.min_button_objects.size());
        for (const auto& min_button_object : parse_result.request.min_button_objects) {
            min_button_objects.push_back({
                .record_index = min_button_object.record_index,
                .object_name = min_button_object.object_name,
                .unique_id = min_button_object.unique_id
            });
        }

        const auto min_button_result = copperfin::vfp::set_visual_object_min_button({
            .path = parse_result.request.path,
            .objects = min_button_objects,
            .min_button = parse_result.request.min_button
        });

        if (!min_button_result.ok) {
            std::cout << "status: error\n";
            std::cout << studio_error_prefix() << min_button_result.error << "\n";
            return 4;
        }
    }
    if (parse_result.request.min_height_object) {
        std::vector<copperfin::vfp::VisualObjectAlignmentTarget> min_height_objects;
        min_height_objects.reserve(parse_result.request.min_height_objects.size());
        for (const auto& min_height_object : parse_result.request.min_height_objects) {
            min_height_objects.push_back({
                .record_index = min_height_object.record_index,
                .object_name = min_height_object.object_name,
                .unique_id = min_height_object.unique_id
            });
        }

        const auto min_height_result = copperfin::vfp::set_visual_object_min_height({
            .path = parse_result.request.path,
            .objects = min_height_objects,
            .min_height = parse_result.request.min_height
        });

        if (!min_height_result.ok) {
            std::cout << "status: error\n";
            std::cout << studio_error_prefix() << min_height_result.error << "\n";
            return 4;
        }
    }
    if (parse_result.request.min_width_object) {
        std::vector<copperfin::vfp::VisualObjectAlignmentTarget> min_width_objects;
        min_width_objects.reserve(parse_result.request.min_width_objects.size());
        for (const auto& min_width_object : parse_result.request.min_width_objects) {
            min_width_objects.push_back({
                .record_index = min_width_object.record_index,
                .object_name = min_width_object.object_name,
                .unique_id = min_width_object.unique_id
            });
        }

        const auto min_width_result = copperfin::vfp::set_visual_object_min_width({
            .path = parse_result.request.path,
            .objects = min_width_objects,
            .min_width = parse_result.request.min_width
        });

        if (!min_width_result.ok) {
            std::cout << "status: error\n";
            std::cout << studio_error_prefix() << min_width_result.error << "\n";
            return 4;
        }
    }
    if (parse_result.request.max_height_object) {
        std::vector<copperfin::vfp::VisualObjectAlignmentTarget> max_height_objects;
        max_height_objects.reserve(parse_result.request.max_height_objects.size());
        for (const auto& max_height_object : parse_result.request.max_height_objects) {
            max_height_objects.push_back({
                .record_index = max_height_object.record_index,
                .object_name = max_height_object.object_name,
                .unique_id = max_height_object.unique_id
            });
        }

        const auto max_height_result = copperfin::vfp::set_visual_object_max_height({
            .path = parse_result.request.path,
            .objects = max_height_objects,
            .max_height = parse_result.request.max_height
        });

        if (!max_height_result.ok) {
            std::cout << "status: error\n";
            std::cout << studio_error_prefix() << max_height_result.error << "\n";
            return 4;
        }
    }
    if (parse_result.request.movable_object) {
        std::vector<copperfin::vfp::VisualObjectAlignmentTarget> movable_objects;
        movable_objects.reserve(parse_result.request.movable_objects.size());
        for (const auto& movable_object : parse_result.request.movable_objects) {
            movable_objects.push_back({
                .record_index = movable_object.record_index,
                .object_name = movable_object.object_name,
                .unique_id = movable_object.unique_id
            });
        }

        const auto movable_result = copperfin::vfp::set_visual_object_movable({
            .path = parse_result.request.path,
            .objects = movable_objects,
            .movable = parse_result.request.movable
        });

        if (!movable_result.ok) {
            std::cout << "status: error\n";
            std::cout << studio_error_prefix() << movable_result.error << "\n";
            return 4;
        }
    }
    if (parse_result.request.half_height_caption_object) {
        std::vector<copperfin::vfp::VisualObjectAlignmentTarget> half_height_caption_objects;
        half_height_caption_objects.reserve(parse_result.request.half_height_caption_objects.size());
        for (const auto& half_height_caption_object : parse_result.request.half_height_caption_objects) {
            half_height_caption_objects.push_back({
                .record_index = half_height_caption_object.record_index,
                .object_name = half_height_caption_object.object_name,
                .unique_id = half_height_caption_object.unique_id
            });
        }

        const auto half_height_caption_result = copperfin::vfp::set_visual_object_half_height_caption({
            .path = parse_result.request.path,
            .objects = half_height_caption_objects,
            .half_height_caption = parse_result.request.half_height_caption
        });

        if (!half_height_caption_result.ok) {
            std::cout << "status: error\n";
            std::cout << studio_error_prefix() << half_height_caption_result.error << "\n";
            return 4;
        }
    }
    if (parse_result.request.mdi_form_object) {
        std::vector<copperfin::vfp::VisualObjectAlignmentTarget> mdi_form_objects;
        mdi_form_objects.reserve(parse_result.request.mdi_form_objects.size());
        for (const auto& mdi_form_object : parse_result.request.mdi_form_objects) {
            mdi_form_objects.push_back({
                .record_index = mdi_form_object.record_index,
                .object_name = mdi_form_object.object_name,
                .unique_id = mdi_form_object.unique_id
            });
        }

        const auto mdi_form_result = copperfin::vfp::set_visual_object_mdi_form({
            .path = parse_result.request.path,
            .objects = mdi_form_objects,
            .mdi_form = parse_result.request.mdi_form
        });

        if (!mdi_form_result.ok) {
            std::cout << "status: error\n";
            std::cout << studio_error_prefix() << mdi_form_result.error << "\n";
            return 4;
        }
    }
    if (parse_result.request.back_style_object) {
        std::vector<copperfin::vfp::VisualObjectAlignmentTarget> back_style_objects;
        back_style_objects.reserve(parse_result.request.back_style_objects.size());
        for (const auto& back_style_object : parse_result.request.back_style_objects) {
            back_style_objects.push_back({
                .record_index = back_style_object.record_index,
                .object_name = back_style_object.object_name,
                .unique_id = back_style_object.unique_id
            });
        }

        const auto back_style_result = copperfin::vfp::set_visual_object_back_style({
            .path = parse_result.request.path,
            .objects = back_style_objects,
            .back_style = parse_result.request.back_style
        });

        if (!back_style_result.ok) {
            std::cout << "status: error\n";
            std::cout << studio_error_prefix() << back_style_result.error << "\n";
            return 4;
        }
    }
    if (parse_result.request.border_style_object) {
        std::vector<copperfin::vfp::VisualObjectAlignmentTarget> border_style_objects;
        border_style_objects.reserve(parse_result.request.border_style_objects.size());
        for (const auto& border_style_object : parse_result.request.border_style_objects) {
            border_style_objects.push_back({
                .record_index = border_style_object.record_index,
                .object_name = border_style_object.object_name,
                .unique_id = border_style_object.unique_id
            });
        }

        const auto border_style_result = copperfin::vfp::set_visual_object_border_style({
            .path = parse_result.request.path,
            .objects = border_style_objects,
            .border_style = parse_result.request.border_style
        });

        if (!border_style_result.ok) {
            std::cout << "status: error\n";
            std::cout << studio_error_prefix() << border_style_result.error << "\n";
            return 4;
        }
    }
    if (parse_result.request.border_width_object) {
        std::vector<copperfin::vfp::VisualObjectAlignmentTarget> border_width_objects;
        border_width_objects.reserve(parse_result.request.border_width_objects.size());
        for (const auto& border_width_object : parse_result.request.border_width_objects) {
            border_width_objects.push_back({
                .record_index = border_width_object.record_index,
                .object_name = border_width_object.object_name,
                .unique_id = border_width_object.unique_id
            });
        }

        const auto border_width_result = copperfin::vfp::set_visual_object_border_width({
            .path = parse_result.request.path,
            .objects = border_width_objects,
            .border_width = parse_result.request.border_width
        });

        if (!border_width_result.ok) {
            std::cout << "status: error\n";
            std::cout << studio_error_prefix() << border_width_result.error << "\n";
            return 4;
        }
    }
    if (parse_result.request.border_color_object) {
        std::vector<copperfin::vfp::VisualObjectAlignmentTarget> border_color_objects;
        border_color_objects.reserve(parse_result.request.border_color_objects.size());
        for (const auto& border_color_object : parse_result.request.border_color_objects) {
            border_color_objects.push_back({
                .record_index = border_color_object.record_index,
                .object_name = border_color_object.object_name,
                .unique_id = border_color_object.unique_id
            });
        }

        const auto border_color_result = copperfin::vfp::set_visual_object_border_color({
            .path = parse_result.request.path,
            .objects = border_color_objects,
            .border_color = parse_result.request.border_color
        });

        if (!border_color_result.ok) {
            std::cout << "status: error\n";
            std::cout << studio_error_prefix() << border_color_result.error << "\n";
            return 4;
        }
    }
    if (parse_result.request.special_effect_object) {
        std::vector<copperfin::vfp::VisualObjectAlignmentTarget> special_effect_objects;
        special_effect_objects.reserve(parse_result.request.special_effect_objects.size());
        for (const auto& special_effect_object : parse_result.request.special_effect_objects) {
            special_effect_objects.push_back({
                .record_index = special_effect_object.record_index,
                .object_name = special_effect_object.object_name,
                .unique_id = special_effect_object.unique_id
            });
        }

        const auto special_effect_result = copperfin::vfp::set_visual_object_special_effect({
            .path = parse_result.request.path,
            .objects = special_effect_objects,
            .special_effect = parse_result.request.special_effect
        });

        if (!special_effect_result.ok) {
            std::cout << "status: error\n";
            std::cout << studio_error_prefix() << special_effect_result.error << "\n";
            return 4;
        }
    }
    if (parse_result.request.scroll_bars_object) {
        std::vector<copperfin::vfp::VisualObjectAlignmentTarget> scroll_bars_objects;
        scroll_bars_objects.reserve(parse_result.request.scroll_bars_objects.size());
        for (const auto& scroll_bars_object : parse_result.request.scroll_bars_objects) {
            scroll_bars_objects.push_back({
                .record_index = scroll_bars_object.record_index,
                .object_name = scroll_bars_object.object_name,
                .unique_id = scroll_bars_object.unique_id
            });
        }

        const auto scroll_bars_result = copperfin::vfp::set_visual_object_scroll_bars({
            .path = parse_result.request.path,
            .objects = scroll_bars_objects,
            .scroll_bars = parse_result.request.scroll_bars
        });

        if (!scroll_bars_result.ok) {
            std::cout << "status: error\n";
            std::cout << studio_error_prefix() << scroll_bars_result.error << "\n";
            return 4;
        }
    }
    if (parse_result.request.window_state_object) {
        std::vector<copperfin::vfp::VisualObjectAlignmentTarget> window_state_objects;
        window_state_objects.reserve(parse_result.request.window_state_objects.size());
        for (const auto& window_state_object : parse_result.request.window_state_objects) {
            window_state_objects.push_back({
                .record_index = window_state_object.record_index,
                .object_name = window_state_object.object_name,
                .unique_id = window_state_object.unique_id
            });
        }

        const auto window_state_result = copperfin::vfp::set_visual_object_window_state({
            .path = parse_result.request.path,
            .objects = window_state_objects,
            .window_state = parse_result.request.window_state
        });

        if (!window_state_result.ok) {
            std::cout << "status: error\n";
            std::cout << studio_error_prefix() << window_state_result.error << "\n";
            return 4;
        }
    }
    if (parse_result.request.show_window_object) {
        std::vector<copperfin::vfp::VisualObjectAlignmentTarget> show_window_objects;
        show_window_objects.reserve(parse_result.request.show_window_objects.size());
        for (const auto& show_window_object : parse_result.request.show_window_objects) {
            show_window_objects.push_back({
                .record_index = show_window_object.record_index,
                .object_name = show_window_object.object_name,
                .unique_id = show_window_object.unique_id
            });
        }

        const auto show_window_result = copperfin::vfp::set_visual_object_show_window({
            .path = parse_result.request.path,
            .objects = show_window_objects,
            .show_window = parse_result.request.show_window
        });

        if (!show_window_result.ok) {
            std::cout << "status: error\n";
            std::cout << studio_error_prefix() << show_window_result.error << "\n";
            return 4;
        }
    }
    if (parse_result.request.title_bar_object) {
        std::vector<copperfin::vfp::VisualObjectAlignmentTarget> title_bar_objects;
        title_bar_objects.reserve(parse_result.request.title_bar_objects.size());
        for (const auto& title_bar_object : parse_result.request.title_bar_objects) {
            title_bar_objects.push_back({
                .record_index = title_bar_object.record_index,
                .object_name = title_bar_object.object_name,
                .unique_id = title_bar_object.unique_id
            });
        }

        const auto title_bar_result = copperfin::vfp::set_visual_object_title_bar({
            .path = parse_result.request.path,
            .objects = title_bar_objects,
            .title_bar = parse_result.request.title_bar
        });

        if (!title_bar_result.ok) {
            std::cout << "status: error\n";
            std::cout << studio_error_prefix() << title_bar_result.error << "\n";
            return 4;
        }
    }
    if (parse_result.request.mouse_pointer_object) {
        std::vector<copperfin::vfp::VisualObjectAlignmentTarget> mouse_pointer_objects;
        mouse_pointer_objects.reserve(parse_result.request.mouse_pointer_objects.size());
        for (const auto& mouse_pointer_object : parse_result.request.mouse_pointer_objects) {
            mouse_pointer_objects.push_back({
                .record_index = mouse_pointer_object.record_index,
                .object_name = mouse_pointer_object.object_name,
                .unique_id = mouse_pointer_object.unique_id
            });
        }

        const auto mouse_pointer_result = copperfin::vfp::set_visual_object_mouse_pointer({
            .path = parse_result.request.path,
            .objects = mouse_pointer_objects,
            .mouse_pointer = parse_result.request.mouse_pointer
        });

        if (!mouse_pointer_result.ok) {
            std::cout << "status: error\n";
            std::cout << studio_error_prefix() << mouse_pointer_result.error << "\n";
            return 4;
        }
    }
    if (parse_result.request.picture_margin_object) {
        std::vector<copperfin::vfp::VisualObjectAlignmentTarget> picture_margin_objects;
        picture_margin_objects.reserve(parse_result.request.picture_margin_objects.size());
        for (const auto& picture_margin_object : parse_result.request.picture_margin_objects) {
            picture_margin_objects.push_back({
                .record_index = picture_margin_object.record_index,
                .object_name = picture_margin_object.object_name,
                .unique_id = picture_margin_object.unique_id
            });
        }

        const auto picture_margin_result = copperfin::vfp::set_visual_object_picture_margin({
            .path = parse_result.request.path,
            .objects = picture_margin_objects,
            .picture_margin = parse_result.request.picture_margin
        });

        if (!picture_margin_result.ok) {
            std::cout << "status: error\n";
            std::cout << studio_error_prefix() << picture_margin_result.error << "\n";
            return 4;
        }
    }
    if (parse_result.request.picture_position_object) {
        std::vector<copperfin::vfp::VisualObjectAlignmentTarget> picture_position_objects;
        picture_position_objects.reserve(parse_result.request.picture_position_objects.size());
        for (const auto& picture_position_object : parse_result.request.picture_position_objects) {
            picture_position_objects.push_back({
                .record_index = picture_position_object.record_index,
                .object_name = picture_position_object.object_name,
                .unique_id = picture_position_object.unique_id
            });
        }

        const auto picture_position_result = copperfin::vfp::set_visual_object_picture_position({
            .path = parse_result.request.path,
            .objects = picture_position_objects,
            .picture_position = parse_result.request.picture_position
        });

        if (!picture_position_result.ok) {
            std::cout << "status: error\n";
            std::cout << studio_error_prefix() << picture_position_result.error << "\n";
            return 4;
        }
    }
    if (parse_result.request.picture_spacing_object) {
        std::vector<copperfin::vfp::VisualObjectAlignmentTarget> picture_spacing_objects;
        picture_spacing_objects.reserve(parse_result.request.picture_spacing_objects.size());
        for (const auto& picture_spacing_object : parse_result.request.picture_spacing_objects) {
            picture_spacing_objects.push_back({
                .record_index = picture_spacing_object.record_index,
                .object_name = picture_spacing_object.object_name,
                .unique_id = picture_spacing_object.unique_id
            });
        }

        const auto picture_spacing_result = copperfin::vfp::set_visual_object_picture_spacing({
            .path = parse_result.request.path,
            .objects = picture_spacing_objects,
            .picture_spacing = parse_result.request.picture_spacing
        });

        if (!picture_spacing_result.ok) {
            std::cout << "status: error\n";
            std::cout << studio_error_prefix() << picture_spacing_result.error << "\n";
            return 4;
        }
    }
    if (parse_result.request.picture_selection_display_object) {
        std::vector<copperfin::vfp::VisualObjectAlignmentTarget> picture_selection_display_objects;
        picture_selection_display_objects.reserve(parse_result.request.picture_selection_display_objects.size());
        for (const auto& picture_selection_display_object : parse_result.request.picture_selection_display_objects) {
            picture_selection_display_objects.push_back({
                .record_index = picture_selection_display_object.record_index,
                .object_name = picture_selection_display_object.object_name,
                .unique_id = picture_selection_display_object.unique_id
            });
        }

        const auto picture_selection_display_result = copperfin::vfp::set_visual_object_picture_selection_display({
            .path = parse_result.request.path,
            .objects = picture_selection_display_objects,
            .picture_selection_display = parse_result.request.picture_selection_display
        });

        if (!picture_selection_display_result.ok) {
            std::cout << "status: error\n";
            std::cout << studio_error_prefix() << picture_selection_display_result.error << "\n";
            return 4;
        }
    }
    if (parse_result.request.dynamic_input_mask_object) {
        std::vector<copperfin::vfp::VisualObjectAlignmentTarget> dynamic_input_mask_objects;
        dynamic_input_mask_objects.reserve(parse_result.request.dynamic_input_mask_objects.size());
        for (const auto& dynamic_input_mask_object : parse_result.request.dynamic_input_mask_objects) {
            dynamic_input_mask_objects.push_back({
                .record_index = dynamic_input_mask_object.record_index,
                .object_name = dynamic_input_mask_object.object_name,
                .unique_id = dynamic_input_mask_object.unique_id
            });
        }

        const auto dynamic_input_mask_result = copperfin::vfp::set_visual_object_dynamic_input_mask({
            .path = parse_result.request.path,
            .objects = dynamic_input_mask_objects,
            .dynamic_input_mask = parse_result.request.dynamic_input_mask
        });

        if (!dynamic_input_mask_result.ok) {
            std::cout << "status: error\n";
            std::cout << studio_error_prefix() << dynamic_input_mask_result.error << "\n";
            return 4;
        }
    }
    if (parse_result.request.dynamic_line_height_object) {
        std::vector<copperfin::vfp::VisualObjectAlignmentTarget> dynamic_line_height_objects;
        dynamic_line_height_objects.reserve(parse_result.request.dynamic_line_height_objects.size());
        for (const auto& dynamic_line_height_object : parse_result.request.dynamic_line_height_objects) {
            dynamic_line_height_objects.push_back({
                .record_index = dynamic_line_height_object.record_index,
                .object_name = dynamic_line_height_object.object_name,
                .unique_id = dynamic_line_height_object.unique_id
            });
        }

        const auto dynamic_line_height_result = copperfin::vfp::set_visual_object_dynamic_line_height({
            .path = parse_result.request.path,
            .objects = dynamic_line_height_objects,
            .dynamic_line_height = parse_result.request.dynamic_line_height
        });

        if (!dynamic_line_height_result.ok) {
            std::cout << "status: error\n";
            std::cout << studio_error_prefix() << dynamic_line_height_result.error << "\n";
            return 4;
        }
    }
    if (parse_result.request.dynamic_alignment_object) {
        std::vector<copperfin::vfp::VisualObjectAlignmentTarget> dynamic_alignment_objects;
        dynamic_alignment_objects.reserve(parse_result.request.dynamic_alignment_objects.size());
        for (const auto& dynamic_alignment_object : parse_result.request.dynamic_alignment_objects) {
            dynamic_alignment_objects.push_back({
                .record_index = dynamic_alignment_object.record_index,
                .object_name = dynamic_alignment_object.object_name,
                .unique_id = dynamic_alignment_object.unique_id
            });
        }

        const auto dynamic_alignment_result = copperfin::vfp::set_visual_object_dynamic_alignment({
            .path = parse_result.request.path,
            .objects = dynamic_alignment_objects,
            .dynamic_alignment = parse_result.request.dynamic_alignment
        });

        if (!dynamic_alignment_result.ok) {
            std::cout << "status: error\n";
            std::cout << studio_error_prefix() << dynamic_alignment_result.error << "\n";
            return 4;
        }
    }
    if (parse_result.request.dynamic_current_control_object) {
        std::vector<copperfin::vfp::VisualObjectAlignmentTarget> dynamic_current_control_objects;
        dynamic_current_control_objects.reserve(parse_result.request.dynamic_current_control_objects.size());
        for (const auto& dynamic_current_control_object : parse_result.request.dynamic_current_control_objects) {
            dynamic_current_control_objects.push_back({
                .record_index = dynamic_current_control_object.record_index,
                .object_name = dynamic_current_control_object.object_name,
                .unique_id = dynamic_current_control_object.unique_id
            });
        }

        const auto dynamic_current_control_result = copperfin::vfp::set_visual_object_dynamic_current_control({
            .path = parse_result.request.path,
            .objects = dynamic_current_control_objects,
            .dynamic_current_control = parse_result.request.dynamic_current_control
        });

        if (!dynamic_current_control_result.ok) {
            std::cout << "status: error\n";
            std::cout << studio_error_prefix() << dynamic_current_control_result.error << "\n";
            return 4;
        }
    }
    if (parse_result.request.dynamic_font_name_object) {
        std::vector<copperfin::vfp::VisualObjectAlignmentTarget> dynamic_font_name_objects;
        dynamic_font_name_objects.reserve(parse_result.request.dynamic_font_name_objects.size());
        for (const auto& dynamic_font_name_object : parse_result.request.dynamic_font_name_objects) {
            dynamic_font_name_objects.push_back({
                .record_index = dynamic_font_name_object.record_index,
                .object_name = dynamic_font_name_object.object_name,
                .unique_id = dynamic_font_name_object.unique_id
            });
        }

        const auto dynamic_font_name_result = copperfin::vfp::set_visual_object_dynamic_font_name({
            .path = parse_result.request.path,
            .objects = dynamic_font_name_objects,
            .dynamic_font_name = parse_result.request.dynamic_font_name
        });

        if (!dynamic_font_name_result.ok) {
            std::cout << "status: error\n";
            std::cout << studio_error_prefix() << dynamic_font_name_result.error << "\n";
            return 4;
        }
    }
    if (parse_result.request.dynamic_font_size_object) {
        std::vector<copperfin::vfp::VisualObjectAlignmentTarget> dynamic_font_size_objects;
        dynamic_font_size_objects.reserve(parse_result.request.dynamic_font_size_objects.size());
        for (const auto& dynamic_font_size_object : parse_result.request.dynamic_font_size_objects) {
            dynamic_font_size_objects.push_back({
                .record_index = dynamic_font_size_object.record_index,
                .object_name = dynamic_font_size_object.object_name,
                .unique_id = dynamic_font_size_object.unique_id
            });
        }

        const auto dynamic_font_size_result = copperfin::vfp::set_visual_object_dynamic_font_size({
            .path = parse_result.request.path,
            .objects = dynamic_font_size_objects,
            .dynamic_font_size = parse_result.request.dynamic_font_size
        });

        if (!dynamic_font_size_result.ok) {
            std::cout << "status: error\n";
            std::cout << studio_error_prefix() << dynamic_font_size_result.error << "\n";
            return 4;
        }
    }
    if (parse_result.request.dynamic_font_bold_object) {
        std::vector<copperfin::vfp::VisualObjectAlignmentTarget> dynamic_font_bold_objects;
        dynamic_font_bold_objects.reserve(parse_result.request.dynamic_font_bold_objects.size());
        for (const auto& dynamic_font_bold_object : parse_result.request.dynamic_font_bold_objects) {
            dynamic_font_bold_objects.push_back({
                .record_index = dynamic_font_bold_object.record_index,
                .object_name = dynamic_font_bold_object.object_name,
                .unique_id = dynamic_font_bold_object.unique_id
            });
        }

        const auto dynamic_font_bold_result = copperfin::vfp::set_visual_object_dynamic_font_bold({
            .path = parse_result.request.path,
            .objects = dynamic_font_bold_objects,
            .dynamic_font_bold = parse_result.request.dynamic_font_bold
        });

        if (!dynamic_font_bold_result.ok) {
            std::cout << "status: error\n";
            std::cout << studio_error_prefix() << dynamic_font_bold_result.error << "\n";
            return 4;
        }
    }
    if (parse_result.request.dynamic_font_italic_object) {
        std::vector<copperfin::vfp::VisualObjectAlignmentTarget> dynamic_font_italic_objects;
        dynamic_font_italic_objects.reserve(parse_result.request.dynamic_font_italic_objects.size());
        for (const auto& dynamic_font_italic_object : parse_result.request.dynamic_font_italic_objects) {
            dynamic_font_italic_objects.push_back({
                .record_index = dynamic_font_italic_object.record_index,
                .object_name = dynamic_font_italic_object.object_name,
                .unique_id = dynamic_font_italic_object.unique_id
            });
        }

        const auto dynamic_font_italic_result = copperfin::vfp::set_visual_object_dynamic_font_italic({
            .path = parse_result.request.path,
            .objects = dynamic_font_italic_objects,
            .dynamic_font_italic = parse_result.request.dynamic_font_italic
        });

        if (!dynamic_font_italic_result.ok) {
            std::cout << "status: error\n";
            std::cout << studio_error_prefix() << dynamic_font_italic_result.error << "\n";
            return 4;
        }
    }
    if (parse_result.request.dynamic_font_underline_object) {
        std::vector<copperfin::vfp::VisualObjectAlignmentTarget> dynamic_font_underline_objects;
        dynamic_font_underline_objects.reserve(parse_result.request.dynamic_font_underline_objects.size());
        for (const auto& dynamic_font_underline_object : parse_result.request.dynamic_font_underline_objects) {
            dynamic_font_underline_objects.push_back({
                .record_index = dynamic_font_underline_object.record_index,
                .object_name = dynamic_font_underline_object.object_name,
                .unique_id = dynamic_font_underline_object.unique_id
            });
        }

        const auto dynamic_font_underline_result = copperfin::vfp::set_visual_object_dynamic_font_underline({
            .path = parse_result.request.path,
            .objects = dynamic_font_underline_objects,
            .dynamic_font_underline = parse_result.request.dynamic_font_underline
        });

        if (!dynamic_font_underline_result.ok) {
            std::cout << "status: error\n";
            std::cout << studio_error_prefix() << dynamic_font_underline_result.error << "\n";
            return 4;
        }
    }
    if (parse_result.request.dynamic_font_strikethru_object) {
        std::vector<copperfin::vfp::VisualObjectAlignmentTarget> dynamic_font_strikethru_objects;
        dynamic_font_strikethru_objects.reserve(parse_result.request.dynamic_font_strikethru_objects.size());
        for (const auto& dynamic_font_strikethru_object : parse_result.request.dynamic_font_strikethru_objects) {
            dynamic_font_strikethru_objects.push_back({
                .record_index = dynamic_font_strikethru_object.record_index,
                .object_name = dynamic_font_strikethru_object.object_name,
                .unique_id = dynamic_font_strikethru_object.unique_id
            });
        }

        const auto dynamic_font_strikethru_result = copperfin::vfp::set_visual_object_dynamic_font_strikethru({
            .path = parse_result.request.path,
            .objects = dynamic_font_strikethru_objects,
            .dynamic_font_strikethru = parse_result.request.dynamic_font_strikethru
        });

        if (!dynamic_font_strikethru_result.ok) {
            std::cout << "status: error\n";
            std::cout << studio_error_prefix() << dynamic_font_strikethru_result.error << "\n";
            return 4;
        }
    }
    if (parse_result.request.dynamic_font_outline_object) {
        std::vector<copperfin::vfp::VisualObjectAlignmentTarget> dynamic_font_outline_objects;
        dynamic_font_outline_objects.reserve(parse_result.request.dynamic_font_outline_objects.size());
        for (const auto& dynamic_font_outline_object : parse_result.request.dynamic_font_outline_objects) {
            dynamic_font_outline_objects.push_back({
                .record_index = dynamic_font_outline_object.record_index,
                .object_name = dynamic_font_outline_object.object_name,
                .unique_id = dynamic_font_outline_object.unique_id
            });
        }

        const auto dynamic_font_outline_result = copperfin::vfp::set_visual_object_dynamic_font_outline({
            .path = parse_result.request.path,
            .objects = dynamic_font_outline_objects,
            .dynamic_font_outline = parse_result.request.dynamic_font_outline
        });

        if (!dynamic_font_outline_result.ok) {
            std::cout << "status: error\n";
            std::cout << studio_error_prefix() << dynamic_font_outline_result.error << "\n";
            return 4;
        }
    }
    if (parse_result.request.dynamic_font_shadow_object) {
        std::vector<copperfin::vfp::VisualObjectAlignmentTarget> dynamic_font_shadow_objects;
        dynamic_font_shadow_objects.reserve(parse_result.request.dynamic_font_shadow_objects.size());
        for (const auto& dynamic_font_shadow_object : parse_result.request.dynamic_font_shadow_objects) {
            dynamic_font_shadow_objects.push_back({
                .record_index = dynamic_font_shadow_object.record_index,
                .object_name = dynamic_font_shadow_object.object_name,
                .unique_id = dynamic_font_shadow_object.unique_id
            });
        }

        const auto dynamic_font_shadow_result = copperfin::vfp::set_visual_object_dynamic_font_shadow({
            .path = parse_result.request.path,
            .objects = dynamic_font_shadow_objects,
            .dynamic_font_shadow = parse_result.request.dynamic_font_shadow
        });

        if (!dynamic_font_shadow_result.ok) {
            std::cout << "status: error\n";
            std::cout << studio_error_prefix() << dynamic_font_shadow_result.error << "\n";
            return 4;
        }
    }
    if (parse_result.request.font_name_object) {
        std::vector<copperfin::vfp::VisualObjectAlignmentTarget> font_name_objects;
        font_name_objects.reserve(parse_result.request.font_name_objects.size());
        for (const auto& font_name_object : parse_result.request.font_name_objects) {
            font_name_objects.push_back({
                .record_index = font_name_object.record_index,
                .object_name = font_name_object.object_name,
                .unique_id = font_name_object.unique_id
            });
        }

        const auto font_name_result = copperfin::vfp::set_visual_object_font_name({
            .path = parse_result.request.path,
            .objects = font_name_objects,
            .font_name = parse_result.request.font_name
        });

        if (!font_name_result.ok) {
            std::cout << "status: error\n";
            std::cout << studio_error_prefix() << font_name_result.error << "\n";
            return 4;
        }
    }
    if (parse_result.request.font_size_object) {
        std::vector<copperfin::vfp::VisualObjectAlignmentTarget> font_size_objects;
        font_size_objects.reserve(parse_result.request.font_size_objects.size());
        for (const auto& font_size_object : parse_result.request.font_size_objects) {
            font_size_objects.push_back({
                .record_index = font_size_object.record_index,
                .object_name = font_size_object.object_name,
                .unique_id = font_size_object.unique_id
            });
        }

        const auto font_size_result = copperfin::vfp::set_visual_object_font_size({
            .path = parse_result.request.path,
            .objects = font_size_objects,
            .font_size = parse_result.request.font_size
        });

        if (!font_size_result.ok) {
            std::cout << "status: error\n";
            std::cout << studio_error_prefix() << font_size_result.error << "\n";
            return 4;
        }
    }
    if (parse_result.request.font_bold_object) {
        std::vector<copperfin::vfp::VisualObjectAlignmentTarget> font_bold_objects;
        font_bold_objects.reserve(parse_result.request.font_bold_objects.size());
        for (const auto& font_bold_object : parse_result.request.font_bold_objects) {
            font_bold_objects.push_back({
                .record_index = font_bold_object.record_index,
                .object_name = font_bold_object.object_name,
                .unique_id = font_bold_object.unique_id
            });
        }

        const auto font_bold_result = copperfin::vfp::set_visual_object_font_bold({
            .path = parse_result.request.path,
            .objects = font_bold_objects,
            .font_bold = parse_result.request.font_bold
        });

        if (!font_bold_result.ok) {
            std::cout << "status: error\n";
            std::cout << studio_error_prefix() << font_bold_result.error << "\n";
            return 4;
        }
    }
    if (parse_result.request.font_italic_object) {
        std::vector<copperfin::vfp::VisualObjectAlignmentTarget> font_italic_objects;
        font_italic_objects.reserve(parse_result.request.font_italic_objects.size());
        for (const auto& font_italic_object : parse_result.request.font_italic_objects) {
            font_italic_objects.push_back({
                .record_index = font_italic_object.record_index,
                .object_name = font_italic_object.object_name,
                .unique_id = font_italic_object.unique_id
            });
        }

        const auto font_italic_result = copperfin::vfp::set_visual_object_font_italic({
            .path = parse_result.request.path,
            .objects = font_italic_objects,
            .font_italic = parse_result.request.font_italic
        });

        if (!font_italic_result.ok) {
            std::cout << "status: error\n";
            std::cout << studio_error_prefix() << font_italic_result.error << "\n";
            return 4;
        }
    }
    if (parse_result.request.font_underline_object) {
        std::vector<copperfin::vfp::VisualObjectAlignmentTarget> font_underline_objects;
        font_underline_objects.reserve(parse_result.request.font_underline_objects.size());
        for (const auto& font_underline_object : parse_result.request.font_underline_objects) {
            font_underline_objects.push_back({
                .record_index = font_underline_object.record_index,
                .object_name = font_underline_object.object_name,
                .unique_id = font_underline_object.unique_id
            });
        }

        const auto font_underline_result = copperfin::vfp::set_visual_object_font_underline({
            .path = parse_result.request.path,
            .objects = font_underline_objects,
            .font_underline = parse_result.request.font_underline
        });

        if (!font_underline_result.ok) {
            std::cout << "status: error\n";
            std::cout << studio_error_prefix() << font_underline_result.error << "\n";
            return 4;
        }
    }
    if (parse_result.request.font_strikethru_object) {
        std::vector<copperfin::vfp::VisualObjectAlignmentTarget> font_strikethru_objects;
        font_strikethru_objects.reserve(parse_result.request.font_strikethru_objects.size());
        for (const auto& font_strikethru_object : parse_result.request.font_strikethru_objects) {
            font_strikethru_objects.push_back({
                .record_index = font_strikethru_object.record_index,
                .object_name = font_strikethru_object.object_name,
                .unique_id = font_strikethru_object.unique_id
            });
        }

        const auto font_strikethru_result = copperfin::vfp::set_visual_object_font_strikethru({
            .path = parse_result.request.path,
            .objects = font_strikethru_objects,
            .font_strikethru = parse_result.request.font_strikethru
        });

        if (!font_strikethru_result.ok) {
            std::cout << "status: error\n";
            std::cout << studio_error_prefix() << font_strikethru_result.error << "\n";
            return 4;
        }
    }
    if (parse_result.request.font_outline_object) {
        std::vector<copperfin::vfp::VisualObjectAlignmentTarget> font_outline_objects;
        font_outline_objects.reserve(parse_result.request.font_outline_objects.size());
        for (const auto& font_outline_object : parse_result.request.font_outline_objects) {
            font_outline_objects.push_back({
                .record_index = font_outline_object.record_index,
                .object_name = font_outline_object.object_name,
                .unique_id = font_outline_object.unique_id
            });
        }

        const auto font_outline_result = copperfin::vfp::set_visual_object_font_outline({
            .path = parse_result.request.path,
            .objects = font_outline_objects,
            .font_outline = parse_result.request.font_outline
        });

        if (!font_outline_result.ok) {
            std::cout << "status: error\n";
            std::cout << studio_error_prefix() << font_outline_result.error << "\n";
            return 4;
        }
    }
    if (parse_result.request.font_shadow_object) {
        std::vector<copperfin::vfp::VisualObjectAlignmentTarget> font_shadow_objects;
        font_shadow_objects.reserve(parse_result.request.font_shadow_objects.size());
        for (const auto& font_shadow_object : parse_result.request.font_shadow_objects) {
            font_shadow_objects.push_back({
                .record_index = font_shadow_object.record_index,
                .object_name = font_shadow_object.object_name,
                .unique_id = font_shadow_object.unique_id
            });
        }

        const auto font_shadow_result = copperfin::vfp::set_visual_object_font_shadow({
            .path = parse_result.request.path,
            .objects = font_shadow_objects,
            .font_shadow = parse_result.request.font_shadow
        });

        if (!font_shadow_result.ok) {
            std::cout << "status: error\n";
            std::cout << studio_error_prefix() << font_shadow_result.error << "\n";
            return 4;
        }
    }
    if (parse_result.request.max_width_object) {
        std::vector<copperfin::vfp::VisualObjectAlignmentTarget> max_width_objects;
        max_width_objects.reserve(parse_result.request.max_width_objects.size());
        for (const auto& max_width_object : parse_result.request.max_width_objects) {
            max_width_objects.push_back({
                .record_index = max_width_object.record_index,
                .object_name = max_width_object.object_name,
                .unique_id = max_width_object.unique_id
            });
        }

        const auto max_width_result = copperfin::vfp::set_visual_object_max_width({
            .path = parse_result.request.path,
            .objects = max_width_objects,
            .max_width = parse_result.request.max_width
        });

        if (!max_width_result.ok) {
            std::cout << "status: error\n";
            std::cout << studio_error_prefix() << max_width_result.error << "\n";
            return 4;
        }
    }
    if (parse_result.request.max_left_object) {
        std::vector<copperfin::vfp::VisualObjectAlignmentTarget> max_left_objects;
        max_left_objects.reserve(parse_result.request.max_left_objects.size());
        for (const auto& max_left_object : parse_result.request.max_left_objects) {
            max_left_objects.push_back({
                .record_index = max_left_object.record_index,
                .object_name = max_left_object.object_name,
                .unique_id = max_left_object.unique_id
            });
        }

        const auto max_left_result = copperfin::vfp::set_visual_object_max_left({
            .path = parse_result.request.path,
            .objects = max_left_objects,
            .max_left = parse_result.request.max_left
        });

        if (!max_left_result.ok) {
            std::cout << "status: error\n";
            std::cout << studio_error_prefix() << max_left_result.error << "\n";
            return 4;
        }
    }
    if (parse_result.request.max_top_object) {
        std::vector<copperfin::vfp::VisualObjectAlignmentTarget> max_top_objects;
        max_top_objects.reserve(parse_result.request.max_top_objects.size());
        for (const auto& max_top_object : parse_result.request.max_top_objects) {
            max_top_objects.push_back({
                .record_index = max_top_object.record_index,
                .object_name = max_top_object.object_name,
                .unique_id = max_top_object.unique_id
            });
        }

        const auto max_top_result = copperfin::vfp::set_visual_object_max_top({
            .path = parse_result.request.path,
            .objects = max_top_objects,
            .max_top = parse_result.request.max_top
        });

        if (!max_top_result.ok) {
            std::cout << "status: error\n";
            std::cout << studio_error_prefix() << max_top_result.error << "\n";
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
            std::cout << studio_error_prefix() << auto_center_result.error << "\n";
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
            std::cout << studio_error_prefix() << auto_size_result.error << "\n";
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
            std::cout << studio_error_prefix() << auto_release_result.error << "\n";
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
            std::cout << studio_error_prefix() << continuous_scroll_result.error << "\n";
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
            std::cout << studio_error_prefix() << dockable_result.error << "\n";
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
            std::cout << studio_error_prefix() << clip_controls_result.error << "\n";
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
            std::cout << studio_error_prefix() << sparse_result.error << "\n";
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
            std::cout << studio_error_prefix() << lock_screen_result.error << "\n";
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
            std::cout << studio_error_prefix() << allow_cell_selection_result.error << "\n";
            return 4;
        }
    }
    if (parse_result.request.hide_selection_object) {
        std::vector<copperfin::vfp::VisualObjectAlignmentTarget> hide_selection_objects;
        hide_selection_objects.reserve(parse_result.request.hide_selection_objects.size());
        for (const auto& hide_selection_object : parse_result.request.hide_selection_objects) {
            hide_selection_objects.push_back({
                .record_index = hide_selection_object.record_index,
                .object_name = hide_selection_object.object_name,
                .unique_id = hide_selection_object.unique_id
            });
        }

        const auto hide_selection_result = copperfin::vfp::set_visual_object_hide_selection({
            .path = parse_result.request.path,
            .objects = hide_selection_objects,
            .hide_selection = parse_result.request.hide_selection
        });

        if (!hide_selection_result.ok) {
            std::cout << "status: error\n";
            std::cout << studio_error_prefix() << hide_selection_result.error << "\n";
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
            std::cout << studio_error_prefix() << delete_mark_result.error << "\n";
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
            std::cout << studio_error_prefix() << record_mark_result.error << "\n";
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
            std::cout << studio_error_prefix() << split_bar_result.error << "\n";
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
            std::cout << studio_error_prefix() << highlight_row_result.error << "\n";
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
            std::cout << studio_error_prefix() << panel_link_result.error << "\n";
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
            std::cout << studio_error_prefix() << allow_header_sizing_result.error << "\n";
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
            std::cout << studio_error_prefix() << allow_row_sizing_result.error << "\n";
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
            std::cout << studio_error_prefix() << resizable_result.error << "\n";
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
            std::cout << studio_error_prefix() << add_line_feeds_result.error << "\n";
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
            std::cout << studio_error_prefix() << always_on_top_result.error << "\n";
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
            std::cout << studio_error_prefix() << always_on_bottom_result.error << "\n";
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
            std::cout << studio_error_prefix() << ungroup_result.error << "\n";
            return 4;
        }
    }
    const auto open_result = copperfin::studio::open_document(open_request, catalog);
    if (!open_result.ok) {
        std::cout << "status: error\n";
        std::cout << studio_error_prefix() << open_result.error << "\n";
        return 3;
    }
    if (parse_result.output_json) {
        print_json_document(open_result.document, catalog, asset_mutation_performed);
        return 0;
    }
    print_document(open_result.document, catalog);
    return 0;
}

#if defined(_WIN32)
int wmain(int argc, wchar_t** argv) {
    std::vector<std::string> utf8_arguments;
    std::vector<char*> narrow_arguments;
    utf8_arguments.reserve(static_cast<std::size_t>(argc));
    narrow_arguments.reserve(static_cast<std::size_t>(argc) + 1U);
    for (int index = 0; index < argc; ++index) {
        utf8_arguments.push_back(
            copperfin::platform::path_to_utf8_string(std::filesystem::path(argv[index])));
    }
    for (auto& argument : utf8_arguments) {
        narrow_arguments.push_back(argument.data());
    }
    narrow_arguments.push_back(nullptr);
    return run_studio_host_main(argc, narrow_arguments.data());
}
#else
int main(int argc, char** argv) {
    return run_studio_host_main(argc, argv);
}
#endif
