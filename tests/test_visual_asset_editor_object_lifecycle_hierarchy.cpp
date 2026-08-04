// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.

#include "test_visual_asset_editor_support.h"

namespace cf_test_visual_asset_editor {
void test_reparent_visual_object_updates_container_parent() {
    namespace fs = std::filesystem;
    const fs::path temp_dir = fs::temp_directory_path() /
        ("copperfin_visual_editor_reparent_tests_" + std::to_string(_getpid()));
    std::error_code ignored;
    fs::remove_all(temp_dir, ignored);
    fs::create_directories(temp_dir);

    const fs::path table_path = temp_dir / "reparent.scx";
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
        {"cntMain", "mainContainer", "container-guid", "frmMain", "container", "container", ""},
        {"cmdSave", "saveButton", "save-guid", "frmMain", "commandbutton", "commandbutton", ""},
        {"txtName", "nameBox", "name-guid", "frmMain", "textbox", "textbox", ""}
    };
    const auto create_result = copperfin::vfp::create_dbf_table_file(table_path.string(), fields, records);
    expect(create_result.ok, "#751: reparent fixture should be writable");

    auto reparent_result = copperfin::vfp::reparent_visual_object({
        .path = table_path.string(),
        .record_index = 0U,
        .object_name = {},
        .unique_id = "save-guid",
        .parent_object_name = "cntMain",
        .parent_unique_id = {},
        .clear_parent = false
    });
    expect(reparent_result.ok, "#751: reparent should support UNIQUEID source and object-name parent selection");
    expect(reparent_result.affected_object_count == 1U,
        "#1006: successful reparent should report one affected object");

    auto parent_result = copperfin::vfp::query_visual_object_property({
        .path = table_path.string(),
        .record_index = 0U,
        .object_name = {},
        .unique_id = "save-guid",
        .property_name = "PARENT"
    });
    expect(parent_result.ok && parent_result.value == "cntMain",
        "#751: reparent should write the resolved parent object name");

    parent_result = copperfin::vfp::query_visual_object_property({
        .path = table_path.string(),
        .record_index = 0U,
        .object_name = {},
        .unique_id = "name-guid",
        .property_name = "PARENT"
    });
    expect(parent_result.ok && parent_result.value == "frmMain",
        "#751: reparent should preserve unrelated object parent fields");

    const auto undo_result = copperfin::vfp::undo_visual_object_property(table_path.string());
    expect(undo_result.ok, "#751: reparent should route through visual property undo");
    parent_result = copperfin::vfp::query_visual_object_property({
        .path = table_path.string(),
        .record_index = 0U,
        .object_name = {},
        .unique_id = "save-guid",
        .property_name = "PARENT"
    });
    expect(parent_result.ok && parent_result.value == "frmMain",
        "#751: undo should restore the previous parent");

    reparent_result = copperfin::vfp::reparent_visual_object({
        .path = table_path.string(),
        .record_index = 0U,
        .object_name = "cmdSave",
        .unique_id = {},
        .parent_object_name = "cntMain",
        .parent_unique_id = {},
        .clear_parent = false
    });
    expect(reparent_result.ok, "#751: reparent should support object-name source selection");

    reparent_result = copperfin::vfp::reparent_visual_object({
        .path = table_path.string(),
        .record_index = 0U,
        .object_name = {},
        .unique_id = "save-guid",
        .parent_object_name = {},
        .parent_unique_id = "form-guid",
        .clear_parent = false
    });
    expect(reparent_result.ok, "#751: reparent should support UNIQUEID parent selection");
    parent_result = copperfin::vfp::query_visual_object_property({
        .path = table_path.string(),
        .record_index = 0U,
        .object_name = {},
        .unique_id = "save-guid",
        .property_name = "PARENT"
    });
    expect(parent_result.ok && parent_result.value == "frmMain",
        "#751: UNIQUEID parent selection should write the target object's OBJNAME");

    reparent_result = copperfin::vfp::reparent_visual_object({
        .path = table_path.string(),
        .record_index = 0U,
        .object_name = {},
        .unique_id = "save-guid",
        .parent_object_name = {},
        .parent_unique_id = {},
        .clear_parent = true
    });
    expect(reparent_result.ok, "#751: reparent should support clearing parent for root-level placement");
    parent_result = copperfin::vfp::query_visual_object_property({
        .path = table_path.string(),
        .record_index = 0U,
        .object_name = {},
        .unique_id = "save-guid",
        .property_name = "PARENT"
    });
    expect(parent_result.ok && parent_result.value.empty(),
        "#751: clear-parent reparent should blank the parent field");

    reparent_result = copperfin::vfp::reparent_visual_object({
        .path = table_path.string(),
        .record_index = 0U,
        .object_name = {},
        .unique_id = "save-guid",
        .parent_object_name = {},
        .parent_unique_id = "save-guid",
        .clear_parent = false
    });
    expect(!reparent_result.ok, "#751: reparent should reject self-parenting");
    expect(reparent_result.affected_object_count == 0U,
        "#1006: failed reparent should report zero affected objects");

    reparent_result = copperfin::vfp::reparent_visual_object({
        .path = table_path.string(),
        .record_index = 0U,
        .object_name = {},
        .unique_id = "save-guid",
        .parent_object_name = "missingParent",
        .parent_unique_id = {},
        .clear_parent = false
    });
    expect(!reparent_result.ok, "#751: reparent should reject missing parent selectors");

    parent_result = copperfin::vfp::query_visual_object_property({
        .path = table_path.string(),
        .record_index = 0U,
        .object_name = {},
        .unique_id = "save-guid",
        .property_name = "PARENT"
    });
    expect(parent_result.ok && parent_result.value.empty(),
        "#751: failed reparent requests should not mutate the selected object's parent");

    fs::remove_all(temp_dir, ignored);
}


void test_reparent_visual_objects_rolls_back_failed_batches() {
    namespace fs = std::filesystem;
    const fs::path temp_dir = fs::temp_directory_path() /
        ("copperfin_visual_editor_reparent_batch_tests_" + std::to_string(_getpid()));
    std::error_code ignored;
    fs::remove_all(temp_dir, ignored);
    fs::create_directories(temp_dir);

    const fs::path table_path = temp_dir / "reparent_batch.scx";
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
        {"cntMain", "mainContainer", "container-guid", "frmMain", "container", "container", ""},
        {"cntAlt", "altContainer", "alt-guid", "frmMain", "container", "container", ""},
        {"cmdSave", "saveButton", "save-guid", "frmMain", "commandbutton", "commandbutton", ""},
        {"txtName", "nameBox", "name-guid", "frmMain", "textbox", "textbox", ""},
        {"lblStatus", "statusLabel", "status-guid", "cntMain", "label", "label", ""}
    };
    const auto create_result = copperfin::vfp::create_dbf_table_file(table_path.string(), fields, records);
    expect(create_result.ok, "#781: reparent-batch fixture should be writable");

    const auto parent_state = [&](const std::string& unique_id) {
        return copperfin::vfp::query_visual_object_property({
            .path = table_path.string(),
            .record_index = 0U,
            .object_name = {},
            .unique_id = unique_id,
            .property_name = "PARENT"
        });
    };

    auto batch_result = copperfin::vfp::reparent_visual_objects({
        .path = table_path.string(),
        .objects = {
            {
                .record_index = 0U,
                .object_name = {},
                .unique_id = "save-guid",
                .parent_object_name = "cntMain",
                .parent_unique_id = {},
                .clear_parent = false
            },
            {
                .record_index = 0U,
                .object_name = "txtName",
                .unique_id = {},
                .parent_object_name = {},
                .parent_unique_id = "alt-guid",
                .clear_parent = false
            },
            {
                .record_index = 5U,
                .object_name = {},
                .unique_id = {},
                .parent_object_name = {},
                .parent_unique_id = {},
                .clear_parent = true
            }
        }
    });
    expect(batch_result.ok, "#781: batch reparent should support mixed selectors, parent names, parent UNIQUEIDs, and clear-parent operations");
    expect(batch_result.affected_object_count == 3U,
        "#1006: successful batch reparent should report affected item count");

    auto save_parent = parent_state("save-guid");
    auto name_parent = parent_state("name-guid");
    auto status_parent = parent_state("status-guid");
    auto container_parent = parent_state("container-guid");
    expect(save_parent.ok && save_parent.value == "cntMain" &&
            name_parent.ok && name_parent.value == "cntAlt" &&
            status_parent.ok && status_parent.value.empty() &&
            container_parent.ok && container_parent.value == "frmMain",
        "#781: batch reparent should persist requested parents while preserving unrelated parent fields");

    const auto undo_before_failure = copperfin::vfp::query_visual_object_undo(table_path.string());
    expect(undo_before_failure.available,
        "#781: successful batch reparents should leave normal visual undo history available");

    batch_result = copperfin::vfp::reparent_visual_objects({
        .path = table_path.string(),
        .objects = {
            {
                .record_index = 0U,
                .object_name = {},
                .unique_id = "save-guid",
                .parent_object_name = "cntAlt",
                .parent_unique_id = {},
                .clear_parent = false
            },
            {
                .record_index = 0U,
                .object_name = {},
                .unique_id = "name-guid",
                .parent_object_name = "missingParent",
                .parent_unique_id = {},
                .clear_parent = false
            }
        }
    });
    expect(!batch_result.ok, "#781: batch reparent should reject missing parent selectors");
    expect(batch_result.affected_object_count == 0U,
        "#1006: failed batch reparent should report zero affected objects");
    save_parent = parent_state("save-guid");
    name_parent = parent_state("name-guid");
    expect(save_parent.ok && save_parent.value == "cntMain" &&
            name_parent.ok && name_parent.value == "cntAlt",
        "#781: missing-parent failures should roll back earlier reparent writes");

    batch_result = copperfin::vfp::reparent_visual_objects({
        .path = table_path.string(),
        .objects = {
            {
                .record_index = 0U,
                .object_name = {},
                .unique_id = "save-guid",
                .parent_object_name = "cntAlt",
                .parent_unique_id = {},
                .clear_parent = false
            },
            {
                .record_index = 0U,
                .object_name = {},
                .unique_id = "missing-guid",
                .parent_object_name = "cntMain",
                .parent_unique_id = {},
                .clear_parent = false
            }
        }
    });
    expect(!batch_result.ok, "#781: batch reparent should reject missing source selectors");
    save_parent = parent_state("save-guid");
    expect(save_parent.ok && save_parent.value == "cntMain",
        "#781: missing-source failures should roll back earlier reparent writes");

    batch_result = copperfin::vfp::reparent_visual_objects({
        .path = table_path.string(),
        .objects = {
            {
                .record_index = 0U,
                .object_name = {},
                .unique_id = "save-guid",
                .parent_object_name = "cntAlt",
                .parent_unique_id = {},
                .clear_parent = false
            },
            {
                .record_index = 0U,
                .object_name = {},
                .unique_id = "container-guid",
                .parent_object_name = {},
                .parent_unique_id = "container-guid",
                .clear_parent = false
            }
        }
    });
    expect(!batch_result.ok, "#781: batch reparent should reject self-parenting");
    save_parent = parent_state("save-guid");
    container_parent = parent_state("container-guid");
    expect(save_parent.ok && save_parent.value == "cntMain" &&
            container_parent.ok && container_parent.value == "frmMain",
        "#781: self-parent failures should roll back earlier reparent writes");

    const auto undo_after_failures = copperfin::vfp::query_visual_object_undo(table_path.string());
    expect(undo_after_failures.available == undo_before_failure.available &&
            undo_after_failures.label == undo_before_failure.label,
        "#781: failed batch reparent rollbacks should preserve prior undo history");

    batch_result = copperfin::vfp::reparent_visual_objects({
        .path = table_path.string(),
        .objects = {}
    });
    expect(!batch_result.ok, "#781: empty batch reparent requests should fail explicitly");
    expect(batch_result.affected_object_count == 0U,
        "#1006: empty batch reparent should report zero affected objects");

    for (int index = 0; index < 3; ++index) {
        const auto undo_result = copperfin::vfp::undo_visual_object_property(table_path.string());
        expect(undo_result.ok, "#781: undo should restore each successful batch reparent write");
    }

    save_parent = parent_state("save-guid");
    name_parent = parent_state("name-guid");
    status_parent = parent_state("status-guid");
    container_parent = parent_state("container-guid");
    expect(save_parent.ok && save_parent.value == "frmMain" &&
            name_parent.ok && name_parent.value == "frmMain" &&
            status_parent.ok && status_parent.value == "cntMain" &&
            container_parent.ok && container_parent.value == "frmMain",
        "#781: successful batch reparent undo should restore original parent state");

    fs::remove_all(temp_dir, ignored);
}


}  // namespace cf_test_visual_asset_editor
