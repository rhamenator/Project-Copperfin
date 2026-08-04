// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only

#include "test_visual_asset_editor_support.h"

namespace cf_test_visual_asset_editor {
void test_list_visual_objects_reads_selection_outline() {
    namespace fs = std::filesystem;
    const fs::path temp_dir = fs::temp_directory_path() /
        ("copperfin_visual_editor_object_outline_tests_" + std::to_string(_getpid()));
    std::error_code ignored;
    fs::remove_all(temp_dir, ignored);
    fs::create_directories(temp_dir);

    const fs::path table_path = temp_dir / "outline.scx";
    const fs::path memo_path = temp_dir / "outline.sct";
    write_synthetic_named_object_asset(table_path, memo_path, {
        {
            .objname = "cmdSave",
            .name = "saveButton",
            .unique_id = "save-guid",
            .properties = "Caption = \"Save\"\r\n"
        },
        {
            .objname = "",
            .name = "fallbackButton",
            .unique_id = "fallback-guid",
            .properties = "Caption = \"Fallback\"\r\n"
        }
    });

    const auto delete_result = copperfin::vfp::set_visual_object_deleted_state({
        .path = table_path.string(),
        .record_index = 0U,
        .object_name = {},
        .unique_id = "fallback-guid",
        .deleted = true
    });
    expect(delete_result.ok, "#743: object outline fixtures should allow marking one row deleted");
    const bool undo_available_after_setup = copperfin::vfp::query_visual_object_undo(table_path.string()).available;
    expect(undo_available_after_setup,
        "#743: marking a row deleted should itself register undo history");

    const auto list_result = copperfin::vfp::list_visual_objects(table_path.string());
    expect(list_result.ok, "#743: visual object outlines should list DBF-family visual assets");
    expect(list_result.objects.size() == 2U, "#743: visual object outlines should include each object record");
    if (list_result.objects.size() == 2U) {
        expect(list_result.objects[0].record_index == 0U && !list_result.objects[0].deleted,
            "#743: visual object outlines should expose live record identity");
        expect(list_result.objects[0].object_name == "cmdSave",
            "#743: visual object outlines should prefer OBJNAME over NAME");
        expect(list_result.objects[0].unique_id == "save-guid",
            "#743: visual object outlines should expose stable UNIQUEID values");
        expect(list_result.objects[0].caption == "\"Save\"",
            "#745: visual object outlines should expose parsed Caption values");
        expect(list_result.objects[1].record_index == 1U && list_result.objects[1].deleted,
            "#743: visual object outlines should keep deleted records visible");
        expect(list_result.objects[1].object_name == "fallbackButton",
            "#743: visual object outlines should fall back to NAME when OBJNAME is absent");
        expect(list_result.objects[1].unique_id == "fallback-guid",
            "#743: visual object outlines should expose stable UNIQUEID values on fallback-name rows");
        expect(list_result.objects[1].caption == "\"Fallback\"",
            "#745: visual object outlines should expose parsed Caption values on fallback-name rows");
    }
    expect(copperfin::vfp::query_visual_object_undo(table_path.string()).available == undo_available_after_setup,
        "#743: read-only visual object outlining should not add to undo history");

    fs::remove_all(temp_dir, ignored);
}

void test_list_visual_objects_reads_hierarchy_metadata() {
    namespace fs = std::filesystem;
    const fs::path temp_dir = fs::temp_directory_path() /
        ("copperfin_visual_editor_object_outline_metadata_tests_" + std::to_string(_getpid()));
    std::error_code ignored;
    fs::remove_all(temp_dir, ignored);
    fs::create_directories(temp_dir);

    const fs::path table_path = temp_dir / "outline_metadata.scx";
    const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
        {.name = "OBJNAME", .type = 'C', .length = 16U},
        {.name = "NAME", .type = 'C', .length = 16U},
        {.name = "UNIQUEID", .type = 'C', .length = 16U},
        {.name = "PARENT", .type = 'C', .length = 16U},
        {.name = "CLASS", .type = 'C', .length = 16U},
        {.name = "BASECLASS", .type = 'C', .length = 16U},
        {.name = "PROPERTIES", .type = 'M', .length = 4U},
        {.name = "METHODS", .type = 'M', .length = 4U}
    };
    const std::vector<std::vector<std::string>> records{
        {
            "Page1",
            "pageOne",
            "page-guid",
            "",
            "pageframe",
            "Page",
            "Caption = \"Page\"\r\nWidth = 200\r\n",
            "PROCEDURE Activate\r\nRETURN\r\n"
        },
        {
            "cmdSave",
            "saveButton",
            "save-guid",
            "Page1",
            "cmdButton",
            "CommandButton",
            "Caption = \"Save\"\r\n",
            "PROCEDURE Click\r\nRETURN\r\nFUNCTION CanSave\r\nRETURN .T.\r\n"
        },
        {"txtName", "nameBox", "name-guid", "page1", "textBox", "TextBox", "", ""}
    };
    const auto create_result = copperfin::vfp::create_dbf_table_file(table_path.string(), fields, records);
    expect(create_result.ok, "#744: object outline metadata fixture should be writable");

    const auto list_result = copperfin::vfp::list_visual_objects(table_path.string());
    expect(list_result.ok, "#744: visual object outlines should list metadata fixtures");
    expect(list_result.objects.size() == 3U, "#744: visual object outlines should preserve row count with metadata fields");
    if (list_result.objects.size() == 3U) {
        expect(!list_result.objects[0].parent_record_available &&
                list_result.objects[0].parent_record_index == 0U &&
                list_result.objects[0].child_count == 2U,
            "#986: root visual object outlines should expose no resolved parent and direct child counts");
        expect(list_result.objects[0].object_path == "Page1" &&
                list_result.objects[0].object_depth == 0U &&
                list_result.objects[0].ancestor_record_indexes.empty() &&
                list_result.objects[0].sibling_index == 0U &&
                list_result.objects[0].sibling_count == 1U,
            "#987: root visual object outlines should expose path, depth, ancestor, and sibling metadata");
        expect(list_result.objects[0].property_count == 10U && list_result.objects[0].method_count == 1U,
            "#988: root visual object outlines should expose direct plus memo property counts and method counts");
        expect(list_result.objects[1].parent_name == "Page1",
            "#744: visual object outlines should expose parent/container names");
        expect(list_result.objects[1].parent_record_available &&
                list_result.objects[1].parent_record_index == 0U &&
                list_result.objects[1].child_count == 0U,
            "#986: child visual object outlines should expose resolved parent record links and leaf counts");
        expect(list_result.objects[1].object_path == "Page1.cmdSave" &&
                list_result.objects[1].object_depth == 1U &&
                list_result.objects[1].ancestor_record_indexes.size() == 1U &&
                list_result.objects[1].ancestor_record_indexes[0] == 0U &&
                list_result.objects[1].sibling_index == 0U &&
                list_result.objects[1].sibling_count == 2U,
            "#987: first child visual object outlines should expose path, depth, ancestors, and sibling order");
        expect(list_result.objects[1].property_count == 9U && list_result.objects[1].method_count == 2U,
            "#988: child visual object outlines should expose parsed property and method counts");
        expect(list_result.objects[1].class_name == "cmdButton",
            "#744: visual object outlines should expose class names");
        expect(list_result.objects[1].baseclass_name == "CommandButton",
            "#744: visual object outlines should expose baseclass names");
        expect(list_result.objects[2].parent_name == "page1" &&
                list_result.objects[2].parent_record_available &&
                list_result.objects[2].parent_record_index == 0U,
            "#986: visual object outline parent resolution should be case-insensitive");
        expect(list_result.objects[2].object_path == "Page1.txtName" &&
                list_result.objects[2].object_depth == 1U &&
                list_result.objects[2].ancestor_record_indexes.size() == 1U &&
                list_result.objects[2].ancestor_record_indexes[0] == 0U &&
                list_result.objects[2].sibling_index == 1U &&
                list_result.objects[2].sibling_count == 2U,
            "#987: second child visual object outlines should expose case-insensitive sibling order");
        expect(list_result.objects[2].property_count == 8U && list_result.objects[2].method_count == 0U,
            "#988: objects with empty property/method memos should still expose direct field counts");
        expect(list_result.objects[2].caption.empty(),
            "#745: visual object outlines should keep captions empty when no Caption property exists");
    }

    const auto children_result = copperfin::vfp::list_visual_object_children({
        .path = table_path.string(),
        .record_index = 0U,
        .object_name = {},
        .unique_id = {}
    });
    expect(children_result.ok && children_result.children.size() == 2U,
        "#986: visual object child listings should preserve direct child snapshots");
    if (children_result.ok && children_result.children.size() == 2U) {
        expect(children_result.children[0].parent_record_available &&
                children_result.children[0].parent_record_index == 0U &&
                children_result.children[0].child_count == 0U &&
                children_result.children[0].object_path == "Page1.cmdSave" &&
                children_result.children[0].sibling_count == 2U &&
                children_result.children[0].property_count == 9U &&
                children_result.children[0].method_count == 2U,
            "#988: embedded child-list snapshots should expose resolved hierarchy and count metadata");
    }

    const auto descendants_result = copperfin::vfp::list_visual_object_descendants({
        .path = table_path.string(),
        .record_index = 0U,
        .object_name = {},
        .unique_id = {}
    });
    expect(descendants_result.ok && descendants_result.descendants.size() == 2U,
        "#986: visual object descendant listings should preserve direct descendant snapshots");
    if (descendants_result.ok && descendants_result.descendants.size() == 2U) {
        expect(descendants_result.descendants[1].object.parent_record_available &&
                descendants_result.descendants[1].object.parent_record_index == 0U &&
                descendants_result.descendants[1].object.child_count == 0U &&
                descendants_result.descendants[1].object.object_path == "Page1.txtName" &&
                descendants_result.descendants[1].object.sibling_index == 1U &&
                descendants_result.descendants[1].object.property_count == 8U &&
                descendants_result.descendants[1].object.method_count == 0U,
            "#988: embedded descendant snapshots should expose resolved hierarchy and count metadata");
    }

    const auto ancestors_result = copperfin::vfp::list_visual_object_ancestors({
        .path = table_path.string(),
        .record_index = 2U,
        .object_name = {},
        .unique_id = {}
    });
    expect(ancestors_result.ok && ancestors_result.ancestors.size() == 1U,
        "#986: visual object ancestor listings should preserve parent snapshots");
    if (ancestors_result.ok && ancestors_result.ancestors.size() == 1U) {
        expect(!ancestors_result.ancestors[0].object.parent_record_available &&
                ancestors_result.ancestors[0].object.child_count == 2U &&
                ancestors_result.ancestors[0].object.object_path == "Page1" &&
                ancestors_result.ancestors[0].object.sibling_count == 1U &&
                ancestors_result.ancestors[0].object.property_count == 10U &&
                ancestors_result.ancestors[0].object.method_count == 1U,
            "#988: embedded ancestor snapshots should expose their own hierarchy and count metadata");
    }

    fs::remove_all(temp_dir, ignored);
}

void test_ungroup_visual_object_reparents_children_and_marks_container_deleted() {
    namespace fs = std::filesystem;
    const fs::path temp_dir = fs::temp_directory_path() /
        ("copperfin_visual_editor_ungroup_tests_" + std::to_string(_getpid()));
    std::error_code ignored;
    fs::remove_all(temp_dir, ignored);
    fs::create_directories(temp_dir);

    const fs::path table_path = temp_dir / "ungroup.scx";
    const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
        {.name = "OBJNAME", .type = 'C', .length = 20U},
        {.name = "NAME", .type = 'C', .length = 20U},
        {.name = "UNIQUEID", .type = 'C', .length = 20U},
        {.name = "PARENT", .type = 'C', .length = 20U},
        {.name = "CLASS", .type = 'C', .length = 20U},
        {.name = "BASECLASS", .type = 'C', .length = 20U},
        {.name = "PROPERTIES", .type = 'M', .length = 4U}
    };
    const std::vector<std::vector<std::string>> records{
        {"frmMain", "mainForm", "form-guid", "", "form", "form", "Caption = \"Main\"\r\n"},
        {"cntGroup", "groupContainer", "group-guid", "frmMain", "container", "container", "Caption = \"Group\"\r\n"},
        {"cmdSave", "saveButton", "save-guid", "cntGroup", "commandbutton", "commandbutton", "Caption = \"Save\"\r\n"},
        {"txtName", "nameBox", "name-guid", "cntGroup", "textbox", "textbox", "Caption = \"Name\"\r\n"},
        {"lblStatus", "statusLabel", "status-guid", "frmMain", "label", "label", "Caption = \"Status\"\r\n"},
        {"cntRoot", "rootContainer", "root-group-guid", "", "container", "container", "Caption = \"Root\"\r\n"},
        {"cmdRoot", "rootButton", "root-child-guid", "cntRoot", "commandbutton", "commandbutton", "Caption = \"Root Child\"\r\n"},
        {"cntEmpty", "emptyContainer", "empty-guid", "frmMain", "container", "container", "Caption = \"Empty\"\r\n"},
        {"", "", "nameless-guid", "frmMain", "container", "container", "Caption = \"Nameless\"\r\n"}
    };
    const auto create_result = copperfin::vfp::create_dbf_table_file(table_path.string(), fields, records);
    expect(create_result.ok, "#789: ungroup fixture should be writable");

    const auto parent_value = [&](const std::string& unique_id) {
        const auto result = copperfin::vfp::query_visual_object_property({
            .path = table_path.string(),
            .record_index = 0U,
            .object_name = {},
            .unique_id = unique_id,
            .property_name = "PARENT"
        });
        expect(result.ok && result.exists, "#789: ungroup parent property should be readable");
        return result.value;
    };
    const auto is_deleted = [&](const std::string& unique_id) {
        const auto list_result = copperfin::vfp::list_visual_objects(table_path.string());
        expect(list_result.ok, "#789: ungroup fixture should remain listable");
        const auto object = std::find_if(
            list_result.objects.begin(),
            list_result.objects.end(),
            [&](const copperfin::vfp::VisualObjectSnapshot& candidate) {
                return candidate.unique_id == unique_id;
            });
        expect(object != list_result.objects.end(), "#789: expected visual object should remain present");
        return object != list_result.objects.end() && object->deleted;
    };

    auto ungroup_result = copperfin::vfp::ungroup_visual_object({
        .path = table_path.string(),
        .record_index = 0U,
        .object_name = {},
        .unique_id = "group-guid"
    });
    expect(ungroup_result.ok &&
            ungroup_result.container_record_index == 1U &&
            ungroup_result.child_count == 2U &&
            ungroup_result.parent_name == "frmMain" &&
            ungroup_result.parent_record_available &&
            ungroup_result.parent_record_index == 0U,
        "#990: nested ungroup should report selected container, child count, and target parent metadata");
    expect(ungroup_result.container_object_name == "cntGroup" &&
            ungroup_result.container_unique_id == "group-guid" &&
            ungroup_result.container_parent_name == "frmMain",
        "#997: nested ungroup should report selected container identity metadata");
    expect(parent_value("save-guid") == "frmMain" &&
            parent_value("name-guid") == "frmMain" &&
            parent_value("status-guid") == "frmMain",
        "#789: ungroup should move children to the container parent and preserve unrelated objects");
    expect(is_deleted("group-guid"),
        "#789: ungroup should mark the container deleted after successful child reparenting");

    auto undo_result = copperfin::vfp::undo_visual_object_property(table_path.string());
    expect(undo_result.ok, "#789: first ungroup reparent write should remain undo-backed");
    undo_result = copperfin::vfp::undo_visual_object_property(table_path.string());
    expect(undo_result.ok, "#789: second ungroup reparent write should remain undo-backed");
    expect(parent_value("save-guid") == "cntGroup" &&
            parent_value("name-guid") == "cntGroup" &&
            is_deleted("group-guid"),
        "#789: ungroup undo should restore child parents while leaving the container deleted");

    ungroup_result = copperfin::vfp::ungroup_visual_object({
        .path = table_path.string(),
        .record_index = 0U,
        .object_name = {},
        .unique_id = "root-group-guid"
    });
    expect(ungroup_result.ok &&
            ungroup_result.child_count == 1U &&
            ungroup_result.parent_name.empty() &&
            !ungroup_result.parent_record_available &&
            ungroup_result.parent_record_index == 0U,
        "#990: root-level ungroup should report empty target-parent metadata");
    expect(parent_value("root-child-guid").empty() && is_deleted("root-group-guid"),
        "#789: root-level ungroup should clear child PARENT values and delete the container");

    const std::string save_parent = parent_value("save-guid");
    const std::string name_parent = parent_value("name-guid");
    ungroup_result = copperfin::vfp::ungroup_visual_object({
        .path = table_path.string(),
        .record_index = 0U,
        .object_name = {},
        .unique_id = "empty-guid"
    });
    expect(!ungroup_result.ok &&
            ungroup_result.parent_name.empty() &&
            !ungroup_result.parent_record_available &&
            ungroup_result.parent_record_index == 0U,
        "#990: empty-container ungroup failures should report empty target-parent metadata");
    expect(ungroup_result.container_object_name.empty() &&
            ungroup_result.container_unique_id.empty() &&
            ungroup_result.container_parent_name.empty(),
        "#997: failed ungroup should not report stale container identity metadata");
    expect(parent_value("save-guid") == save_parent &&
            parent_value("name-guid") == name_parent &&
            !is_deleted("empty-guid"),
        "#789: empty-container failures should not mutate unrelated children or delete the container");

    ungroup_result = copperfin::vfp::ungroup_visual_object({
        .path = table_path.string(),
        .record_index = 0U,
        .object_name = {},
        .unique_id = "nameless-guid"
    });
    expect(!ungroup_result.ok &&
            ungroup_result.parent_name.empty() &&
            !ungroup_result.parent_record_available &&
            ungroup_result.parent_record_index == 0U,
        "#990: nameless-container ungroup failures should report empty target-parent metadata");
    expect(!is_deleted("nameless-guid"),
        "#789: nameless-container failures should not delete the selected row");

    ungroup_result = copperfin::vfp::ungroup_visual_object({
        .path = table_path.string(),
        .record_index = 0U,
        .object_name = {},
        .unique_id = "missing-guid"
    });
    expect(!ungroup_result.ok &&
            ungroup_result.parent_name.empty() &&
            !ungroup_result.parent_record_available &&
            ungroup_result.parent_record_index == 0U,
        "#990: missing-container ungroup failures should report empty target-parent metadata");

    fs::remove_all(temp_dir, ignored);
}

void test_list_visual_object_children_filters_immediate_children() {
    namespace fs = std::filesystem;
    const fs::path temp_dir = fs::temp_directory_path() /
        ("copperfin_visual_editor_children_tests_" + std::to_string(_getpid()));
    std::error_code ignored;
    fs::remove_all(temp_dir, ignored);
    fs::create_directories(temp_dir);

    const fs::path table_path = temp_dir / "children.scx";
    const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
        {.name = "OBJNAME", .type = 'C', .length = 20U},
        {.name = "NAME", .type = 'C', .length = 20U},
        {.name = "UNIQUEID", .type = 'C', .length = 20U},
        {.name = "PARENT", .type = 'C', .length = 20U},
        {.name = "CLASS", .type = 'C', .length = 20U},
        {.name = "BASECLASS", .type = 'C', .length = 20U},
        {.name = "PROPERTIES", .type = 'M', .length = 4U}
    };
    const std::vector<std::vector<std::string>> records{
        {"", "mainForm", "form-guid", "", "form", "form", "Caption = \"Main\"\r\n"},
        {"cmdSave", "saveButton", "save-guid", "mainForm", "commandbutton", "commandbutton", "Caption = \"Save\"\r\n"},
        {"txtName", "nameBox", "name-guid", "mainForm", "textbox", "textbox", "Caption = \"Name\"\r\n"},
        {"lblNested", "nestedLabel", "nested-guid", "cmdSave", "label", "label", "Caption = \"Nested\"\r\n"},
        {"cmdOther", "otherButton", "other-guid", "", "commandbutton", "commandbutton", ""},
        {"", "", "nameless-guid", "", "custom", "custom", ""}
    };
    const auto create_result = copperfin::vfp::create_dbf_table_file(table_path.string(), fields, records);
    expect(create_result.ok, "#756: children fixture should be writable");
    const auto delete_result = copperfin::vfp::set_visual_object_deleted_state({
        .path = table_path.string(),
        .record_index = 0U,
        .object_name = {},
        .unique_id = "name-guid",
        .deleted = true
    });
    expect(delete_result.ok, "#756: children fixture should support deleted child setup");
    const bool undo_available_after_setup = copperfin::vfp::query_visual_object_undo(table_path.string()).available;
    expect(undo_available_after_setup,
        "#756: marking a child deleted should itself register undo history");

    auto children_result = copperfin::vfp::list_visual_object_children({
        .path = table_path.string(),
        .record_index = 0U,
        .object_name = {},
        .unique_id = "form-guid"
    });
    expect(children_result.ok &&
            children_result.parent_record_index == 0U &&
            children_result.parent_name == "mainForm" &&
            children_result.children.size() == 2U,
        "#756: child listing should support UNIQUEID parent selection and fallback parent NAME resolution");
    if (children_result.ok && children_result.children.size() == 2U) {
        expect(children_result.children[0].unique_id == "save-guid" &&
                !children_result.children[0].deleted &&
                children_result.children[0].parent_name == "mainForm" &&
                children_result.children[0].caption == "\"Save\"",
            "#756: child listing should include live immediate children with outline metadata");
        expect(children_result.children[1].unique_id == "name-guid" &&
                children_result.children[1].deleted &&
                children_result.children[1].parent_name == "mainForm" &&
                children_result.children[1].caption == "\"Name\"",
            "#756: child listing should keep deleted immediate children visible");
    }
    const auto has_grandchild = std::find_if(
        children_result.children.begin(),
        children_result.children.end(),
        [](const copperfin::vfp::VisualObjectSnapshot& child) {
            return child.unique_id == "nested-guid";
        });
    expect(has_grandchild == children_result.children.end(),
        "#756: child listing should exclude grandchildren");

    children_result = copperfin::vfp::list_visual_object_children({
        .path = table_path.string(),
        .record_index = 0U,
        .object_name = "mainForm",
        .unique_id = {}
    });
    expect(children_result.ok && children_result.children.size() == 2U,
        "#756: child listing should support fallback NAME parent selection");

    children_result = copperfin::vfp::list_visual_object_children({
        .path = table_path.string(),
        .record_index = 0U,
        .object_name = {},
        .unique_id = "missing-guid"
    });
    expect(!children_result.ok, "#756: child listing should fail explicitly for missing parents");

    children_result = copperfin::vfp::list_visual_object_children({
        .path = table_path.string(),
        .record_index = 0U,
        .object_name = {},
        .unique_id = "nameless-guid"
    });
    expect(!children_result.ok, "#756: child listing should fail explicitly for nameless parent rows");

    expect(copperfin::vfp::query_visual_object_undo(table_path.string()).available == undo_available_after_setup,
        "#756: read-only child listing should not add to undo history");

    fs::remove_all(temp_dir, ignored);
}

void test_list_visual_object_descendants_walks_container_tree() {
    namespace fs = std::filesystem;
    const fs::path temp_dir = fs::temp_directory_path() /
        ("copperfin_visual_editor_descendants_tests_" + std::to_string(_getpid()));
    std::error_code ignored;
    fs::remove_all(temp_dir, ignored);
    fs::create_directories(temp_dir);

    const fs::path table_path = temp_dir / "descendants.scx";
    const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
        {.name = "OBJNAME", .type = 'C', .length = 20U},
        {.name = "NAME", .type = 'C', .length = 20U},
        {.name = "UNIQUEID", .type = 'C', .length = 20U},
        {.name = "PARENT", .type = 'C', .length = 20U},
        {.name = "CLASS", .type = 'C', .length = 20U},
        {.name = "BASECLASS", .type = 'C', .length = 20U},
        {.name = "PROPERTIES", .type = 'M', .length = 4U}
    };
    const std::vector<std::vector<std::string>> records{
        {"", "mainForm", "form-guid", "", "form", "form", "Caption = \"Main\"\r\n"},
        {"cntA", "containerA", "a-guid", "mainForm", "container", "container", "Caption = \"A\"\r\n"},
        {"txtName", "nameBox", "name-guid", "cntA", "textbox", "textbox", "Caption = \"Name\"\r\n"},
        {"lblNested", "nestedLabel", "nested-guid", "txtName", "label", "label", "Caption = \"Nested\"\r\n"},
        {"dupContainer", "dupOne", "dup-one-guid", "mainForm", "container", "container", ""},
        {"dupContainer", "dupTwo", "dup-two-guid", "mainForm", "container", "container", ""},
        {"dupChild", "dupChildName", "dup-child-guid", "dupContainer", "label", "label", ""},
        {"cmdOther", "otherButton", "other-guid", "", "commandbutton", "commandbutton", ""},
        {"", "", "nameless-guid", "", "custom", "custom", ""}
    };
    const auto create_result = copperfin::vfp::create_dbf_table_file(table_path.string(), fields, records);
    expect(create_result.ok, "#757: descendants fixture should be writable");
    const auto delete_result = copperfin::vfp::set_visual_object_deleted_state({
        .path = table_path.string(),
        .record_index = 0U,
        .object_name = {},
        .unique_id = "name-guid",
        .deleted = true
    });
    expect(delete_result.ok, "#757: descendants fixture should support deleted descendant setup");
    const bool undo_available_after_setup = copperfin::vfp::query_visual_object_undo(table_path.string()).available;
    expect(undo_available_after_setup,
        "#757: marking a descendant deleted should itself register undo history");

    auto descendants_result = copperfin::vfp::list_visual_object_descendants({
        .path = table_path.string(),
        .record_index = 0U,
        .object_name = {},
        .unique_id = "form-guid"
    });
    expect(descendants_result.ok &&
            descendants_result.parent_record_index == 0U &&
            descendants_result.parent_name == "mainForm" &&
            descendants_result.descendants.size() == 6U,
        "#757: descendants should support UNIQUEID parent selection and fallback parent NAME resolution");
    if (descendants_result.ok && descendants_result.descendants.size() == 6U) {
        expect(descendants_result.descendants[0].object.unique_id == "a-guid" &&
                descendants_result.descendants[0].depth == 1U,
            "#757: descendants should list immediate children first with depth one");
        expect(descendants_result.descendants[1].object.unique_id == "name-guid" &&
                descendants_result.descendants[1].depth == 2U &&
                descendants_result.descendants[1].object.deleted,
            "#757: descendants should include deleted nested descendants with depth metadata");
        expect(descendants_result.descendants[2].object.unique_id == "nested-guid" &&
                descendants_result.descendants[2].depth == 3U,
            "#757: descendants should walk grandchildren in pre-order");
        expect(descendants_result.descendants[3].object.unique_id == "dup-one-guid" &&
                descendants_result.descendants[4].object.unique_id == "dup-child-guid" &&
                descendants_result.descendants[4].depth == 2U &&
                descendants_result.descendants[5].object.unique_id == "dup-two-guid",
            "#757: descendants should protect duplicate parent-name traversal from duplicate child entries");
    }
    const auto sibling = std::find_if(
        descendants_result.descendants.begin(),
        descendants_result.descendants.end(),
        [](const copperfin::vfp::VisualObjectDescendantSnapshot& descendant) {
            return descendant.object.unique_id == "other-guid";
        });
    expect(sibling == descendants_result.descendants.end(),
        "#757: descendants should exclude sibling/root-level objects outside the selected parent");

    descendants_result = copperfin::vfp::list_visual_object_descendants({
        .path = table_path.string(),
        .record_index = 0U,
        .object_name = "mainForm",
        .unique_id = {}
    });
    expect(descendants_result.ok && descendants_result.descendants.size() == 6U,
        "#757: descendants should support fallback NAME parent selection");

    descendants_result = copperfin::vfp::list_visual_object_descendants({
        .path = table_path.string(),
        .record_index = 0U,
        .object_name = {},
        .unique_id = "missing-guid"
    });
    expect(!descendants_result.ok, "#757: descendants should fail explicitly for missing parents");

    descendants_result = copperfin::vfp::list_visual_object_descendants({
        .path = table_path.string(),
        .record_index = 0U,
        .object_name = {},
        .unique_id = "nameless-guid"
    });
    expect(!descendants_result.ok, "#757: descendants should fail explicitly for nameless parent rows");

    expect(copperfin::vfp::query_visual_object_undo(table_path.string()).available == undo_available_after_setup,
        "#757: read-only descendant listing should not add to undo history");

    fs::remove_all(temp_dir, ignored);
}

void test_set_visual_object_subtree_deleted_state_updates_descendants() {
    namespace fs = std::filesystem;
    const fs::path temp_dir = fs::temp_directory_path() /
        ("copperfin_visual_editor_subtree_delete_tests_" + std::to_string(_getpid()));
    std::error_code ignored;
    fs::remove_all(temp_dir, ignored);
    fs::create_directories(temp_dir);

    const fs::path table_path = temp_dir / "subtree_delete.scx";
    const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
        {.name = "OBJNAME", .type = 'C', .length = 20U},
        {.name = "NAME", .type = 'C', .length = 20U},
        {.name = "UNIQUEID", .type = 'C', .length = 20U},
        {.name = "PARENT", .type = 'C', .length = 20U}
    };
    const std::vector<std::vector<std::string>> records{
        {"cntMain", "mainContainer", "container-guid", ""},
        {"cmdSave", "saveButton", "save-guid", "cntMain"},
        {"txtName", "nameBox", "name-guid", "cntMain"},
        {"lblNested", "nestedLabel", "nested-guid", "txtName"},
        {"cmdOther", "otherButton", "other-guid", ""},
        {"", "", "nameless-guid", ""}
    };
    const auto create_result = copperfin::vfp::create_dbf_table_file(table_path.string(), fields, records);
    expect(create_result.ok, "#758: subtree deleted-state fixture should be writable");
    const auto initial_delete_result = copperfin::vfp::set_visual_object_deleted_state({
        .path = table_path.string(),
        .record_index = 0U,
        .object_name = {},
        .unique_id = "name-guid",
        .deleted = true
    });
    expect(initial_delete_result.ok, "#758: subtree fixture should support existing deleted descendants");

    const auto is_deleted = [&](const std::string& unique_id) {
        const auto list_result = copperfin::vfp::list_visual_objects(table_path.string());
        expect(list_result.ok, "#758: subtree fixture should remain listable");
        const auto object = std::find_if(
            list_result.objects.begin(),
            list_result.objects.end(),
            [&](const copperfin::vfp::VisualObjectSnapshot& candidate) {
                return candidate.unique_id == unique_id;
            });
        expect(object != list_result.objects.end(), "#758: expected subtree object should remain present");
        return object != list_result.objects.end() && object->deleted;
    };

    auto subtree_result = copperfin::vfp::set_visual_object_subtree_deleted_state({
        .path = table_path.string(),
        .record_index = 0U,
        .object_name = {},
        .unique_id = "container-guid",
        .deleted = true
    });
    expect(subtree_result.ok, "#758: subtree deleted-state changes should support UNIQUEID source selection");
    expect(subtree_result.affected_object_count == 3U,
        "#1006: subtree delete should report root plus changed descendant count");
    expect(is_deleted("container-guid") &&
            is_deleted("save-guid") &&
            is_deleted("name-guid") &&
            is_deleted("nested-guid"),
        "#758: subtree delete should mark the selected root and all descendants deleted");
    expect(!is_deleted("other-guid"),
        "#758: subtree delete should preserve unrelated root/sibling rows");

    subtree_result = copperfin::vfp::set_visual_object_subtree_deleted_state({
        .path = table_path.string(),
        .record_index = 0U,
        .object_name = "cntMain",
        .unique_id = {},
        .deleted = false
    });
    expect(subtree_result.ok, "#758: subtree deleted-state changes should support object-name source selection");
    expect(subtree_result.affected_object_count == 4U,
        "#1006: subtree restore should report root plus descendant count");
    expect(!is_deleted("container-guid") &&
            !is_deleted("save-guid") &&
            !is_deleted("name-guid") &&
            !is_deleted("nested-guid"),
        "#758: subtree restore should clear deleted flags on root and descendants");

    subtree_result = copperfin::vfp::set_visual_object_subtree_deleted_state({
        .path = table_path.string(),
        .record_index = 0U,
        .object_name = {},
        .unique_id = "missing-guid",
        .deleted = true
    });
    expect(!subtree_result.ok, "#758: subtree delete should fail explicitly for missing source selections");
    expect(subtree_result.affected_object_count == 0U,
        "#1006: failed subtree delete should report zero affected objects");

    subtree_result = copperfin::vfp::set_visual_object_subtree_deleted_state({
        .path = table_path.string(),
        .record_index = 0U,
        .object_name = {},
        .unique_id = "nameless-guid",
        .deleted = true
    });
    expect(!subtree_result.ok, "#758: subtree delete should fail explicitly for nameless source rows");

    expect(!is_deleted("container-guid") &&
            !is_deleted("save-guid") &&
            !is_deleted("name-guid") &&
            !is_deleted("nested-guid") &&
            !is_deleted("other-guid") &&
            !is_deleted("nameless-guid"),
        "#758: failed subtree deleted-state requests should not mutate existing flags");

    fs::remove_all(temp_dir, ignored);
}

void test_set_visual_object_deleted_state_is_undoable() {
    namespace fs = std::filesystem;
    const fs::path temp_dir = fs::temp_directory_path() /
        ("copperfin_visual_editor_deleted_state_undo_tests_" + std::to_string(_getpid()));
    std::error_code ignored;
    fs::remove_all(temp_dir, ignored);
    fs::create_directories(temp_dir);

    const fs::path table_path = temp_dir / "deleted_state_undo.scx";
    const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
        {.name = "OBJNAME", .type = 'C', .length = 20U},
        {.name = "NAME", .type = 'C', .length = 20U},
        {.name = "UNIQUEID", .type = 'C', .length = 20U},
        {.name = "PARENT", .type = 'C', .length = 20U}
    };
    const std::vector<std::vector<std::string>> records{
        {"cntMain", "mainContainer", "container-guid", ""},
        {"cmdSave", "saveButton", "save-guid", "cntMain"},
        {"txtName", "nameBox", "name-guid", "cntMain"}
    };
    const auto create_result = copperfin::vfp::create_dbf_table_file(table_path.string(), fields, records);
    expect(create_result.ok, "deleted-state undo fixture should be writable");

    const auto is_deleted = [&](const std::string& unique_id) {
        const auto list_result = copperfin::vfp::list_visual_objects(table_path.string());
        expect(list_result.ok, "deleted-state undo fixture should remain listable");
        const auto object = std::find_if(
            list_result.objects.begin(),
            list_result.objects.end(),
            [&](const copperfin::vfp::VisualObjectSnapshot& candidate) {
                return candidate.unique_id == unique_id;
            });
        expect(object != list_result.objects.end(), "expected deleted-state undo object should remain present");
        return object != list_result.objects.end() && object->deleted;
    };

    expect(!copperfin::vfp::query_visual_object_undo(table_path.string()).available,
        "a freshly created asset should have no undo history");

    const auto delete_result = copperfin::vfp::set_visual_object_deleted_state({
        .path = table_path.string(),
        .record_index = 0U,
        .object_name = {},
        .unique_id = "save-guid",
        .deleted = true
    });
    expect(delete_result.ok, "single deleted-state change should succeed");
    expect(is_deleted("save-guid"), "single deleted-state change should mark the record deleted");
    expect(copperfin::vfp::query_visual_object_undo(table_path.string()).available,
        "single deleted-state change should register undo history");

    const auto undo_result = copperfin::vfp::undo_visual_object_property(table_path.string());
    expect(undo_result.ok, "undo after a single deleted-state change should succeed");
    expect(!is_deleted("save-guid"), "undo should restore the record's prior (non-deleted) state");
    expect(!copperfin::vfp::query_visual_object_undo(table_path.string()).available,
        "undo should drain the journal once fully replayed");

    const auto batch_result = copperfin::vfp::set_visual_object_deleted_states({
        .path = table_path.string(),
        .objects = {
            {.record_index = 1U, .object_name = {}, .unique_id = "save-guid", .deleted = true},
            {.record_index = 2U, .object_name = {}, .unique_id = "name-guid", .deleted = true}
        }
    });
    expect(batch_result.ok, "batch deleted-state change should succeed");
    expect(is_deleted("save-guid") && is_deleted("name-guid"),
        "batch deleted-state change should mark all requested records deleted");
    expect(copperfin::vfp::query_visual_object_undo(table_path.string()).available,
        "batch deleted-state change should register undo history");

    const auto batch_undo_result = copperfin::vfp::undo_visual_object_property(table_path.string());
    expect(batch_undo_result.ok, "undo after a batch deleted-state change should succeed");
    expect(!is_deleted("save-guid") && !is_deleted("name-guid"),
        "undo should restore every record the batch change touched");

    fs::remove_all(temp_dir, ignored);
}

void test_list_visual_object_ancestors_walks_parent_chain() {
    namespace fs = std::filesystem;
    const fs::path temp_dir = fs::temp_directory_path() /
        ("copperfin_visual_editor_ancestors_tests_" + std::to_string(_getpid()));
    std::error_code ignored;
    fs::remove_all(temp_dir, ignored);
    fs::create_directories(temp_dir);

    const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
        {.name = "OBJNAME", .type = 'C', .length = 20U},
        {.name = "NAME", .type = 'C', .length = 20U},
        {.name = "UNIQUEID", .type = 'C', .length = 20U},
        {.name = "PARENT", .type = 'C', .length = 20U},
        {.name = "CLASS", .type = 'C', .length = 20U},
        {.name = "BASECLASS", .type = 'C', .length = 20U},
        {.name = "PROPERTIES", .type = 'M', .length = 4U}
    };

    const fs::path table_path = temp_dir / "ancestors.scx";
    const std::vector<std::vector<std::string>> records{
        {"frmMain", "mainForm", "form-guid", "", "form", "form", "Caption = \"Main\"\r\n"},
        {"cntMain", "mainContainer", "container-guid", "frmMain", "container", "container", "Caption = \"Container\"\r\n"},
        {"pgDetails", "detailsPage", "page-guid", "cntMain", "page", "page", "Caption = \"Details\"\r\n"},
        {"cmdSave", "saveButton", "save-guid", "pgDetails", "commandbutton", "commandbutton", "Caption = \"Save\"\r\n"},
        {"cmdOther", "otherButton", "other-guid", "", "commandbutton", "commandbutton", ""}
    };
    const auto create_result = copperfin::vfp::create_dbf_table_file(table_path.string(), fields, records);
    expect(create_result.ok, "#759: ancestors fixture should be writable");

    auto ancestors_result = copperfin::vfp::list_visual_object_ancestors({
        .path = table_path.string(),
        .record_index = 0U,
        .object_name = {},
        .unique_id = "save-guid"
    });
    expect(ancestors_result.ok &&
            ancestors_result.record_index == 3U &&
            ancestors_result.ancestors.size() == 3U,
        "#759: ancestors should support UNIQUEID source selection");
    if (ancestors_result.ok && ancestors_result.ancestors.size() == 3U) {
        expect(ancestors_result.ancestors[0].object.unique_id == "page-guid" &&
                ancestors_result.ancestors[0].depth == 1U &&
                ancestors_result.ancestors[0].object.caption == "\"Details\"",
            "#759: ancestors should list the immediate parent first with outline metadata");
        expect(ancestors_result.ancestors[1].object.unique_id == "container-guid" &&
                ancestors_result.ancestors[1].depth == 2U,
            "#759: ancestors should include intermediate containers with depth metadata");
        expect(ancestors_result.ancestors[2].object.unique_id == "form-guid" &&
                ancestors_result.ancestors[2].depth == 3U,
            "#759: ancestors should walk upward to the root object");
    }

    ancestors_result = copperfin::vfp::list_visual_object_ancestors({
        .path = table_path.string(),
        .record_index = 0U,
        .object_name = "cmdSave",
        .unique_id = {}
    });
    expect(ancestors_result.ok && ancestors_result.ancestors.size() == 3U,
        "#759: ancestors should support object-name source selection");

    ancestors_result = copperfin::vfp::list_visual_object_ancestors({
        .path = table_path.string(),
        .record_index = 0U,
        .object_name = {},
        .unique_id = "form-guid"
    });
    expect(ancestors_result.ok && ancestors_result.ancestors.empty(),
        "#759: root objects should return an empty successful ancestor list");

    ancestors_result = copperfin::vfp::list_visual_object_ancestors({
        .path = table_path.string(),
        .record_index = 0U,
        .object_name = {},
        .unique_id = "missing-guid"
    });
    expect(!ancestors_result.ok, "#759: ancestors should fail explicitly for missing source objects");

    const fs::path ambiguous_path = temp_dir / "ambiguous_ancestor.scx";
    const std::vector<std::vector<std::string>> ambiguous_records{
        {"dupParent", "parentOne", "parent-one-guid", "", "container", "container", ""},
        {"dupParent", "parentTwo", "parent-two-guid", "", "container", "container", ""},
        {"cmdChild", "childButton", "child-guid", "dupParent", "commandbutton", "commandbutton", ""}
    };
    const auto ambiguous_create_result = copperfin::vfp::create_dbf_table_file(
        ambiguous_path.string(),
        fields,
        ambiguous_records);
    expect(ambiguous_create_result.ok, "#759: ambiguous ancestor fixture should be writable");
    ancestors_result = copperfin::vfp::list_visual_object_ancestors({
        .path = ambiguous_path.string(),
        .record_index = 0U,
        .object_name = {},
        .unique_id = "child-guid"
    });
    expect(!ancestors_result.ok, "#759: ancestors should reject ambiguous parent names");

    const fs::path cycle_path = temp_dir / "cycle_ancestor.scx";
    const std::vector<std::vector<std::string>> cycle_records{
        {"cntA", "containerA", "a-guid", "cntB", "container", "container", ""},
        {"cntB", "containerB", "b-guid", "cntA", "container", "container", ""}
    };
    const auto cycle_create_result = copperfin::vfp::create_dbf_table_file(
        cycle_path.string(),
        fields,
        cycle_records);
    expect(cycle_create_result.ok, "#759: cycle ancestor fixture should be writable");
    ancestors_result = copperfin::vfp::list_visual_object_ancestors({
        .path = cycle_path.string(),
        .record_index = 0U,
        .object_name = {},
        .unique_id = "a-guid"
    });
    expect(!ancestors_result.ok, "#759: ancestors should reject parent cycles");

    expect(!copperfin::vfp::query_visual_object_undo(table_path.string()).available,
        "#759: ancestor listing should not create undo history");

    fs::remove_all(temp_dir, ignored);
}

}  // namespace cf_test_visual_asset_editor
