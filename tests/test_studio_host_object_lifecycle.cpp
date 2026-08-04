// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.

#include "test_studio_host_support.h"

namespace cf_test_studio_host {
void test_parse_launch_arguments_for_delete_object() {
    const auto result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--json",
        "--delete-object",
        "--object-name", "txtName",
        "--unique-id", "textbox-guid"
    });

    expect(result.ok, "#1023: launch contract should parse delete-object requests");
    expect(result.request.delete_object, "#1023: launch contract should detect --delete-object");
    expect(result.request.object_name == "txtName",
        "#1023: delete-object requests should carry object-name selectors");
    expect(result.request.unique_id == "textbox-guid",
        "#1023: delete-object requests should carry unique-id selectors");
}

void test_parse_launch_arguments_rejects_delete_object_property_ambiguity() {
    const auto result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--delete-object",
        "--clear-property",
        "--property-name", "Caption"
    });

    expect(!result.ok,
        "#1023: launch contract should reject delete-object combined with property commands");
}

void test_parse_launch_arguments_for_restore_object() {
    const auto result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--json",
        "--restore-object",
        "--object-name", "txtName",
        "--unique-id", "textbox-guid"
    });

    expect(result.ok, "#1024: launch contract should parse restore-object requests");
    expect(result.request.restore_object, "#1024: launch contract should detect --restore-object");
    expect(result.request.object_name == "txtName",
        "#1024: restore-object requests should carry object-name selectors");
    expect(result.request.unique_id == "textbox-guid",
        "#1024: restore-object requests should carry unique-id selectors");
}

void test_parse_launch_arguments_rejects_restore_object_ambiguity() {
    const auto delete_restore_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--delete-object",
        "--restore-object",
        "--unique-id", "textbox-guid"
    });
    expect(!delete_restore_result.ok,
        "#1024: launch contract should reject simultaneous delete-object and restore-object requests");

    const auto restore_property_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--restore-object",
        "--clear-property",
        "--property-name", "Caption"
    });
    expect(!restore_property_result.ok,
        "#1024: launch contract should reject restore-object combined with property commands");
}

void test_parse_launch_arguments_for_deleted_states() {
    const auto result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--json",
        "--deleted-states",
        "--deleted-state-target-object-name", "cmdSave",
        "--deleted-state", "true",
        "--deleted-state-target-unique-id", "name-guid",
        "--deleted-state", "false"
    });

    expect(result.ok, "#1201: launch contract should parse deleted-states requests");
    expect(result.request.deleted_states, "#1201: launch contract should detect --deleted-states");
    expect(result.request.deleted_state_objects.size() == 2U,
        "#1201: deleted-states requests should collect target/state pairs");
    if (result.request.deleted_state_objects.size() == 2U) {
        expect(result.request.deleted_state_objects[0].object_name == "cmdSave" &&
                result.request.deleted_state_objects[0].unique_id.empty() &&
                result.request.deleted_state_objects[0].deleted_available &&
                result.request.deleted_state_objects[0].deleted,
            "#1201: deleted-states requests should parse object-name delete items");
        expect(result.request.deleted_state_objects[1].object_name.empty() &&
                result.request.deleted_state_objects[1].unique_id == "name-guid" &&
                result.request.deleted_state_objects[1].deleted_available &&
                !result.request.deleted_state_objects[1].deleted,
            "#1201: deleted-states requests should parse unique-id restore items");
    }
}

void test_parse_launch_arguments_rejects_deleted_states_invalid_inputs() {
    const auto missing_state_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--deleted-states",
        "--deleted-state-target-unique-id", "one-guid"
    });
    expect(!missing_state_result.ok,
        "#1201: launch contract should reject deleted-states items without deleted state");

    const auto missing_targets_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--deleted-states"
    });
    expect(!missing_targets_result.ok,
        "#1201: launch contract should reject deleted-states requests without target selectors");

    const auto invalid_value_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--deleted-states",
        "--deleted-state-target-unique-id", "one-guid",
        "--deleted-state", "sometimes"
    });
    expect(!invalid_value_result.ok,
        "#1201: launch contract should reject invalid deleted-state logical values");

    const auto value_without_target_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--deleted-states",
        "--deleted-state", "true"
    });
    expect(!value_without_target_result.ok,
        "#1201: launch contract should reject deleted-state values without preceding target selectors");

    const auto stray_target_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--deleted-state-target-unique-id", "one-guid",
        "--deleted-state", "true"
    });
    expect(!stray_target_result.ok,
        "#1201: launch contract should reject stray deleted-state target arguments");
}

void test_parse_launch_arguments_rejects_deleted_states_ambiguity() {
    const auto deleted_states_delete_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--deleted-states",
        "--deleted-state-target-unique-id", "one-guid",
        "--deleted-state", "true",
        "--delete-object",
        "--unique-id", "one-guid"
    });
    expect(!deleted_states_delete_result.ok,
        "#1201: launch contract should reject simultaneous deleted-states and delete-object requests");

    const auto deleted_states_property_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--deleted-states",
        "--deleted-state-target-unique-id", "one-guid",
        "--deleted-state", "true",
        "--clear-property",
        "--property-name", "Caption"
    });
    expect(!deleted_states_property_result.ok,
        "#1201: launch contract should reject deleted-states combined with property commands");
}

void test_parse_launch_arguments_for_subtree_deleted_state() {
    const auto result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--json",
        "--subtree-deleted-state",
        "--subtree-deleted", "true",
        "--object-name", "cntMain",
        "--unique-id", "container-guid"
    });

    expect(result.ok, "#1202: launch contract should parse subtree deleted-state requests");
    expect(result.request.subtree_deleted_state,
        "#1202: launch contract should detect --subtree-deleted-state");
    expect(result.request.subtree_deleted_available && result.request.subtree_deleted,
        "#1202: launch contract should parse subtree deleted-state logical values");
    expect(result.request.object_name == "cntMain" &&
            result.request.unique_id == "container-guid",
        "#1202: subtree deleted-state requests should preserve root selectors");
}

void test_parse_launch_arguments_rejects_subtree_deleted_state_invalid_inputs() {
    const auto missing_state_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--subtree-deleted-state",
        "--unique-id", "container-guid"
    });
    expect(!missing_state_result.ok,
        "#1202: launch contract should reject subtree deleted-state requests without state");

    const auto missing_selector_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--subtree-deleted-state",
        "--subtree-deleted", "true"
    });
    expect(!missing_selector_result.ok,
        "#1202: launch contract should reject subtree deleted-state requests without root selectors");

    const auto invalid_value_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--subtree-deleted-state",
        "--unique-id", "container-guid",
        "--subtree-deleted", "sometimes"
    });
    expect(!invalid_value_result.ok,
        "#1202: launch contract should reject invalid subtree deleted-state logical values");

    const auto stray_state_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--subtree-deleted", "true",
        "--unique-id", "container-guid"
    });
    expect(!stray_state_result.ok,
        "#1202: launch contract should reject stray subtree deleted-state arguments");
}

void test_parse_launch_arguments_rejects_subtree_deleted_state_ambiguity() {
    const auto subtree_delete_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--subtree-deleted-state",
        "--subtree-deleted", "true",
        "--unique-id", "container-guid",
        "--delete-object"
    });
    expect(!subtree_delete_result.ok,
        "#1202: launch contract should reject simultaneous subtree deleted-state and delete-object requests");

    const auto subtree_property_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--subtree-deleted-state",
        "--subtree-deleted", "true",
        "--unique-id", "container-guid",
        "--clear-property",
        "--property-name", "Caption"
    });
    expect(!subtree_property_result.ok,
        "#1202: launch contract should reject subtree deleted-state combined with property commands");
}

void test_parse_launch_arguments_for_duplicate_object() {
    const auto result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--json",
        "--duplicate-object",
        "--object-name", "txtName",
        "--unique-id", "textbox-guid",
        "--new-object-name", "txtNameCopy",
        "--new-name", "txtNameCopy",
        "--new-unique-id", "textbox-copy-guid"
    });

    expect(result.ok, "#1025: launch contract should parse duplicate-object requests");
    expect(result.request.duplicate_object, "#1025: launch contract should detect --duplicate-object");
    expect(result.request.object_name == "txtName",
        "#1025: duplicate-object requests should carry object-name selectors");
    expect(result.request.unique_id == "textbox-guid",
        "#1025: duplicate-object requests should carry unique-id selectors");
    expect(result.request.new_object_name == "txtNameCopy",
        "#1025: duplicate-object requests should carry replacement object names");
    expect(result.request.new_name == "txtNameCopy",
        "#1025: duplicate-object requests should carry replacement NAME values");
    expect(result.request.new_unique_id == "textbox-copy-guid",
        "#1025: duplicate-object requests should carry replacement unique ids");
}

void test_parse_launch_arguments_rejects_duplicate_object_ambiguity() {
    const auto duplicate_delete_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--duplicate-object",
        "--delete-object",
        "--unique-id", "textbox-guid"
    });
    expect(!duplicate_delete_result.ok,
        "#1025: launch contract should reject simultaneous duplicate-object and delete-object requests");

    const auto duplicate_property_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--duplicate-object",
        "--clear-property",
        "--property-name", "Caption"
    });
    expect(!duplicate_property_result.ok,
        "#1025: launch contract should reject duplicate-object combined with property commands");
}

void test_parse_launch_arguments_for_rename_object() {
    const auto result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--json",
        "--rename-object",
        "--object-name", "txtName",
        "--unique-id", "textbox-guid",
        "--new-object-name", "txtCustomer",
        "--new-name", "txtCustomer",
        "--new-unique-id", "customer-textbox-guid"
    });

    expect(result.ok, "#1026: launch contract should parse rename-object requests");
    expect(result.request.rename_object, "#1026: launch contract should detect --rename-object");
    expect(result.request.object_name == "txtName",
        "#1026: rename-object requests should carry object-name selectors");
    expect(result.request.unique_id == "textbox-guid",
        "#1026: rename-object requests should carry unique-id selectors");
    expect(result.request.new_object_name == "txtCustomer",
        "#1026: rename-object requests should carry replacement object names");
    expect(result.request.new_name == "txtCustomer",
        "#1026: rename-object requests should carry replacement NAME values");
    expect(result.request.new_unique_id == "customer-textbox-guid",
        "#1026: rename-object requests should carry replacement unique ids");
}

void test_parse_launch_arguments_rejects_rename_object_ambiguity_and_empty_identity() {
    const auto empty_identity_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--rename-object",
        "--unique-id", "textbox-guid"
    });
    expect(!empty_identity_result.ok,
        "#1026: launch contract should reject rename-object requests without replacement identity fields");

    const auto rename_duplicate_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--rename-object",
        "--duplicate-object",
        "--unique-id", "textbox-guid",
        "--new-object-name", "txtCustomer"
    });
    expect(!rename_duplicate_result.ok,
        "#1026: launch contract should reject simultaneous rename-object and duplicate-object requests");

    const auto rename_property_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--rename-object",
        "--clear-property",
        "--property-name", "Caption",
        "--new-object-name", "txtCustomer"
    });
    expect(!rename_property_result.ok,
        "#1026: launch contract should reject rename-object combined with property commands");
}

void test_parse_launch_arguments_for_reparent_object() {
    const auto result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--json",
        "--reparent-object",
        "--object-name", "txtName",
        "--unique-id", "textbox-guid",
        "--parent-name", "cntPanel",
        "--parent-unique-id", "panel-guid"
    });

    expect(result.ok, "#1027: launch contract should parse reparent-object requests");
    expect(result.request.reparent_object, "#1027: launch contract should detect --reparent-object");
    expect(result.request.object_name == "txtName",
        "#1027: reparent-object requests should carry source object-name selectors");
    expect(result.request.unique_id == "textbox-guid",
        "#1027: reparent-object requests should carry source unique-id selectors");
    expect(result.request.parent_name == "cntPanel",
        "#1027: reparent-object requests should carry parent object-name selectors");
    expect(result.request.parent_unique_id == "panel-guid",
        "#1027: reparent-object requests should carry parent unique-id selectors");
}

void test_parse_launch_arguments_rejects_reparent_object_ambiguity_and_missing_parent() {
    const auto missing_parent_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--reparent-object",
        "--unique-id", "textbox-guid"
    });
    expect(!missing_parent_result.ok,
        "#1027: launch contract should reject reparent-object requests without parent selectors or clear-parent");

    const auto clear_parent_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--reparent-object",
        "--unique-id", "textbox-guid",
        "--clear-parent"
    });
    expect(clear_parent_result.ok && clear_parent_result.request.clear_parent,
        "#1027: launch contract should parse clear-parent reparent requests");

    const auto reparent_rename_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--reparent-object",
        "--rename-object",
        "--unique-id", "textbox-guid",
        "--parent-name", "cntPanel",
        "--new-object-name", "txtCustomer"
    });
    expect(!reparent_rename_result.ok,
        "#1027: launch contract should reject simultaneous reparent-object and rename-object requests");

    const auto reparent_property_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--reparent-object",
        "--clear-property",
        "--property-name", "Caption",
        "--parent-name", "cntPanel"
    });
    expect(!reparent_property_result.ok,
        "#1027: launch contract should reject reparent-object combined with property commands");
}

void test_parse_launch_arguments_for_reorder_object() {
    const auto result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--json",
        "--reorder-object",
        "--object-name", "txtName",
        "--unique-id", "textbox-guid",
        "--placement", "before",
        "--target-object-name", "cmdSave",
        "--target-unique-id", "button-guid"
    });

    expect(result.ok, "#1028: launch contract should parse reorder-object requests");
    expect(result.request.reorder_object, "#1028: launch contract should detect --reorder-object");
    expect(result.request.object_name == "txtName",
        "#1028: reorder-object requests should carry source object-name selectors");
    expect(result.request.unique_id == "textbox-guid",
        "#1028: reorder-object requests should carry source unique-id selectors");
    expect(result.request.placement == "before",
        "#1028: reorder-object requests should carry placement");
    expect(result.request.target_object_name == "cmdSave",
        "#1028: reorder-object requests should carry target object-name selectors");
    expect(result.request.target_unique_id == "button-guid",
        "#1028: reorder-object requests should carry target unique-id selectors");
}

void test_parse_launch_arguments_rejects_reorder_object_ambiguity_and_missing_placement() {
    const auto missing_placement_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--reorder-object",
        "--unique-id", "textbox-guid"
    });
    expect(!missing_placement_result.ok,
        "#1028: launch contract should reject reorder-object requests without placement");

    const auto reorder_reparent_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--reorder-object",
        "--reparent-object",
        "--unique-id", "textbox-guid",
        "--placement", "front",
        "--parent-name", "cntPanel"
    });
    expect(!reorder_reparent_result.ok,
        "#1028: launch contract should reject simultaneous reorder-object and reparent-object requests");

    const auto reorder_property_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--reorder-object",
        "--clear-property",
        "--property-name", "Caption",
        "--placement", "front"
    });
    expect(!reorder_property_result.ok,
        "#1028: launch contract should reject reorder-object combined with property commands");
}

void test_parse_launch_arguments_for_ungroup_object() {
    const auto result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--json",
        "--ungroup-object",
        "--object-name", "cntGroup",
        "--unique-id", "group-guid"
    });

    expect(result.ok, "#1029: launch contract should parse ungroup-object requests");
    expect(result.request.ungroup_object, "#1029: launch contract should detect --ungroup-object");
    expect(result.request.object_name == "cntGroup",
        "#1029: ungroup-object requests should carry source object-name selectors");
    expect(result.request.unique_id == "group-guid",
        "#1029: ungroup-object requests should carry source unique-id selectors");
}

void test_parse_launch_arguments_rejects_ungroup_object_ambiguity() {
    const auto ungroup_reorder_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--ungroup-object",
        "--reorder-object",
        "--unique-id", "group-guid",
        "--placement", "front"
    });
    expect(!ungroup_reorder_result.ok,
        "#1029: launch contract should reject simultaneous ungroup-object and reorder-object requests");

    const auto ungroup_property_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--ungroup-object",
        "--clear-property",
        "--property-name", "Caption"
    });
    expect(!ungroup_property_result.ok,
        "#1029: launch contract should reject ungroup-object combined with property commands");
}

void test_parse_launch_arguments_for_group_object() {
    const auto result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--json",
        "--group-object",
        "--field-value", "OBJNAME=cntGroup",
        "--field-value", "UNIQUEID=group-guid",
        "--field-value", "PARENT=frmCustomer",
        "--group-child-object-name", "cmdSave",
        "--group-child-unique-id", "name-guid"
    });

    expect(result.ok, "#1030: launch contract should parse group-object requests");
    expect(result.request.group_object, "#1030: launch contract should detect --group-object");
    expect(result.request.field_values.size() == 3U,
        "#1030: group-object requests should collect container field values");
    if (result.request.field_values.size() == 3U) {
        expect(result.request.field_values[0].property_name == "OBJNAME" &&
                result.request.field_values[0].property_value == "cntGroup",
            "#1030: group-object requests should parse first container field assignment");
        expect(result.request.field_values[1].property_name == "UNIQUEID" &&
                result.request.field_values[1].property_value == "group-guid",
            "#1030: group-object requests should parse second container field assignment");
    }
    expect(result.request.group_objects.size() == 2U,
        "#1030: group-object requests should collect grouped child selectors");
    if (result.request.group_objects.size() == 2U) {
        expect(result.request.group_objects[0].object_name == "cmdSave" &&
                result.request.group_objects[0].unique_id.empty(),
            "#1030: group-object requests should parse child object-name selectors");
        expect(result.request.group_objects[1].object_name.empty() &&
                result.request.group_objects[1].unique_id == "name-guid",
            "#1030: group-object requests should parse child unique-id selectors");
    }
}

void test_parse_launch_arguments_rejects_group_object_invalid_inputs() {
    const auto missing_field_values_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--group-object",
        "--group-child-unique-id", "name-guid"
    });
    expect(!missing_field_values_result.ok,
        "#1030: launch contract should reject group-object requests without container field values");

    const auto missing_children_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--group-object",
        "--field-value", "OBJNAME=cntGroup"
    });
    expect(!missing_children_result.ok,
        "#1030: launch contract should reject group-object requests without child selectors");

    const auto invalid_assignment_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--group-object",
        "--field-value", "OBJNAME",
        "--group-child-unique-id", "name-guid"
    });
    expect(!invalid_assignment_result.ok,
        "#1030: launch contract should reject group-object field values without assignment syntax");
}

void test_parse_launch_arguments_rejects_group_object_ambiguity() {
    const auto group_ungroup_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--group-object",
        "--ungroup-object",
        "--field-value", "OBJNAME=cntGroup",
        "--group-child-unique-id", "name-guid"
    });
    expect(!group_ungroup_result.ok,
        "#1030: launch contract should reject simultaneous group-object and ungroup-object requests");

    const auto group_property_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--group-object",
        "--clear-property",
        "--property-name", "Caption",
        "--field-value", "OBJNAME=cntGroup",
        "--group-child-unique-id", "name-guid"
    });
    expect(!group_property_result.ok,
        "#1030: launch contract should reject group-object combined with property commands");

    const auto stray_field_value_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--field-value", "OBJNAME=cntGroup"
    });
    expect(!stray_field_value_result.ok,
        "#1030: launch contract should reject stray group field values");
}

}  // namespace cf_test_studio_host
