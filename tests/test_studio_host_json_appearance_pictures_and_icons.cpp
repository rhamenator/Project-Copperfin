// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only

#include "test_studio_host_json_support.h"

namespace cf_test_studio_host_json {
void write_synthetic_form_table_for_object_picture(const std::filesystem::path& form_path) {
    const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
        {.name = "OBJNAME", .type = 'C', .length = 24U},
        {.name = "NAME", .type = 'C', .length = 24U},
        {.name = "UNIQUEID", .type = 'C', .length = 32U},
        {.name = "PICTURE", .type = 'C', .length = 64U}
    };
    const std::vector<std::vector<std::string>> records{
        {"cmdSave", "cmdSave", "one-guid", "forms\\save.bmp"},
        {"cmdCancel", "cmdCancel", "two-guid", "forms\\cancel.bmp"},
        {"lblStatus", "lblStatus", "three-guid", "forms\\status.bmp"},
        {"cmdOther", "cmdOther", "other-guid", "forms\\other.bmp"}
    };

    const auto create_result = copperfin::vfp::create_dbf_table_file(form_path.string(), fields, records);
    expect(create_result.ok, "#1098: synthetic SCX table for object picture should be created");
}

void write_synthetic_form_table_for_object_down_picture(const std::filesystem::path& form_path) {
    const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
        {.name = "OBJNAME", .type = 'C', .length = 24U},
        {.name = "NAME", .type = 'C', .length = 24U},
        {.name = "UNIQUEID", .type = 'C', .length = 32U},
        {.name = "DOWNPICTUR", .type = 'C', .length = 64U}
    };
    const std::vector<std::vector<std::string>> records{
        {"cmdSave", "cmdSave", "one-guid", "forms\\save_down.bmp"},
        {"cmdCancel", "cmdCancel", "two-guid", "forms\\cancel_down.bmp"},
        {"lblStatus", "lblStatus", "three-guid", "forms\\status_down.bmp"},
        {"cmdOther", "cmdOther", "other-guid", "forms\\other_down.bmp"}
    };

    const auto create_result = copperfin::vfp::create_dbf_table_file(form_path.string(), fields, records);
    expect(create_result.ok, "#1099: synthetic SCX table for object down-picture should be created");
}

void write_synthetic_form_table_for_object_disabled_picture(const std::filesystem::path& form_path) {
    const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
        {.name = "OBJNAME", .type = 'C', .length = 24U},
        {.name = "NAME", .type = 'C', .length = 24U},
        {.name = "UNIQUEID", .type = 'C', .length = 32U},
        {.name = "DISABLEDPI", .type = 'C', .length = 64U}
    };
    const std::vector<std::vector<std::string>> records{
        {"cmdSave", "cmdSave", "one-guid", "forms\\save_disabled.bmp"},
        {"cmdCancel", "cmdCancel", "two-guid", "forms\\cancel_disabled.bmp"},
        {"lblStatus", "lblStatus", "three-guid", "forms\\status_disabled.bmp"},
        {"cmdOther", "cmdOther", "other-guid", "forms\\other_disabled.bmp"}
    };

    const auto create_result = copperfin::vfp::create_dbf_table_file(form_path.string(), fields, records);
    expect(create_result.ok, "#1100: synthetic SCX table for object disabled-picture should be created");
}

void write_synthetic_form_table_for_object_ole_drag_picture(const std::filesystem::path& form_path) {
    const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
        {.name = "OBJNAME", .type = 'C', .length = 24U},
        {.name = "NAME", .type = 'C', .length = 24U},
        {.name = "UNIQUEID", .type = 'C', .length = 32U},
        {.name = "OLEDRAGPIC", .type = 'C', .length = 64U}
    };
    const std::vector<std::vector<std::string>> records{
        {"cmdSave", "cmdSave", "one-guid", "forms\\save_ole_drag.bmp"},
        {"cmdCancel", "cmdCancel", "two-guid", "forms\\cancel_ole_drag.bmp"},
        {"lblStatus", "lblStatus", "three-guid", "forms\\status_ole_drag.bmp"},
        {"cmdOther", "cmdOther", "other-guid", "forms\\other_ole_drag.bmp"}
    };

    const auto create_result = copperfin::vfp::create_dbf_table_file(form_path.string(), fields, records);
    expect(create_result.ok, "#1101: synthetic SCX table for object OLE drag-picture should be created");
}

void write_synthetic_form_table_for_object_mouse_icon(const std::filesystem::path& form_path) {
    const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
        {.name = "OBJNAME", .type = 'C', .length = 24U},
        {.name = "NAME", .type = 'C', .length = 24U},
        {.name = "UNIQUEID", .type = 'C', .length = 32U},
        {.name = "MOUSEICON", .type = 'C', .length = 64U}
    };
    const std::vector<std::vector<std::string>> records{
        {"cmdSave", "cmdSave", "one-guid", "forms\\save_mouse.cur"},
        {"cmdCancel", "cmdCancel", "two-guid", "forms\\cancel_mouse.cur"},
        {"lblStatus", "lblStatus", "three-guid", "forms\\status_mouse.cur"},
        {"cmdOther", "cmdOther", "other-guid", "forms\\other_mouse.cur"}
    };

    const auto create_result = copperfin::vfp::create_dbf_table_file(form_path.string(), fields, records);
    expect(create_result.ok, "#1102: synthetic SCX table for object mouse-icon should be created");
}

void write_synthetic_form_table_for_object_drag_icon(const std::filesystem::path& form_path) {
    const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
        {.name = "OBJNAME", .type = 'C', .length = 24U},
        {.name = "NAME", .type = 'C', .length = 24U},
        {.name = "UNIQUEID", .type = 'C', .length = 32U},
        {.name = "DRAGICON", .type = 'C', .length = 64U}
    };
    const std::vector<std::vector<std::string>> records{
        {"cmdSave", "cmdSave", "one-guid", "forms\\save_drag.cur"},
        {"cmdCancel", "cmdCancel", "two-guid", "forms\\cancel_drag.cur"},
        {"lblStatus", "lblStatus", "three-guid", "forms\\status_drag.cur"},
        {"cmdOther", "cmdOther", "other-guid", "forms\\other_drag.cur"}
    };

    const auto create_result = copperfin::vfp::create_dbf_table_file(form_path.string(), fields, records);
    expect(create_result.ok, "#1103: synthetic SCX table for object drag-icon should be created");
}

void write_synthetic_form_table_for_object_picture_margin(const std::filesystem::path& form_path) {
    const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
        {.name = "OBJNAME", .type = 'C', .length = 24U},
        {.name = "NAME", .type = 'C', .length = 24U},
        {.name = "UNIQUEID", .type = 'C', .length = 32U},
        {.name = "PICTUREMAR", .type = 'N', .length = 10U, .decimal_count = 0U}
    };
    const std::vector<std::vector<std::string>> records{
        {"frmCustomer", "frmCustomer", "one-guid", "0"},
        {"frmOrder", "frmOrder", "two-guid", "0"},
        {"cntDetails", "cntDetails", "three-guid", "1"},
        {"frmOther", "frmOther", "other-guid", "1"}
    };

    const auto create_result = copperfin::vfp::create_dbf_table_file(form_path.string(), fields, records);
    expect(create_result.ok, "#1172: synthetic SCX table for object picture margin should be created");
}

void write_synthetic_form_table_for_object_picture_position(const std::filesystem::path& form_path) {
    const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
        {.name = "OBJNAME", .type = 'C', .length = 24U},
        {.name = "NAME", .type = 'C', .length = 24U},
        {.name = "UNIQUEID", .type = 'C', .length = 32U},
        {.name = "PICTUREPOS", .type = 'N', .length = 10U, .decimal_count = 0U}
    };
    const std::vector<std::vector<std::string>> records{
        {"frmCustomer", "frmCustomer", "one-guid", "0"},
        {"frmOrder", "frmOrder", "two-guid", "0"},
        {"cntDetails", "cntDetails", "three-guid", "1"},
        {"frmOther", "frmOther", "other-guid", "1"}
    };

    const auto create_result = copperfin::vfp::create_dbf_table_file(form_path.string(), fields, records);
    expect(create_result.ok, "#1173: synthetic SCX table for object picture position should be created");
}

void write_synthetic_form_table_for_object_picture_spacing(const std::filesystem::path& form_path) {
    const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
        {.name = "OBJNAME", .type = 'C', .length = 24U},
        {.name = "NAME", .type = 'C', .length = 24U},
        {.name = "UNIQUEID", .type = 'C', .length = 32U},
        {.name = "PICTURESPA", .type = 'N', .length = 10U, .decimal_count = 0U}
    };
    const std::vector<std::vector<std::string>> records{
        {"frmCustomer", "frmCustomer", "one-guid", "0"},
        {"frmOrder", "frmOrder", "two-guid", "0"},
        {"cntDetails", "cntDetails", "three-guid", "1"},
        {"frmOther", "frmOther", "other-guid", "1"}
    };

    const auto create_result = copperfin::vfp::create_dbf_table_file(form_path.string(), fields, records);
    expect(create_result.ok, "#1174: synthetic SCX table for object picture spacing should be created");
}

void write_synthetic_form_table_for_object_picture_selection_display(const std::filesystem::path& form_path) {
    const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
        {.name = "OBJNAME", .type = 'C', .length = 24U},
        {.name = "NAME", .type = 'C', .length = 24U},
        {.name = "UNIQUEID", .type = 'C', .length = 32U},
        {.name = "PICTURESEL", .type = 'N', .length = 10U, .decimal_count = 0U}
    };
    const std::vector<std::vector<std::string>> records{
        {"frmCustomer", "frmCustomer", "one-guid", "0"},
        {"frmOrder", "frmOrder", "two-guid", "0"},
        {"cntDetails", "cntDetails", "three-guid", "1"},
        {"frmOther", "frmOther", "other-guid", "1"}
    };

    const auto create_result = copperfin::vfp::create_dbf_table_file(form_path.string(), fields, records);
    expect(create_result.ok, "#1175: synthetic SCX table for object picture selection display should be created");
}

void test_studio_host_json_assigns_picture_by_stable_selectors(const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root = fs::temp_directory_path() / "copperfin_studio_host_picture_object_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path picture_path = temp_root / "picture.scx";
    write_synthetic_form_table_for_object_picture(picture_path);
    const auto picture_process = run_process_capture(
        studio_host_path,
        {
            "--path", picture_path.string(),
            "--picture-object",
            "--picture", "forms\\customer hero.bmp",
            "--picture-target-object-name", "cmdSave",
            "--picture-target-unique-id", "two-guid",
            "--json"
        },
        temp_root);
    expect(picture_process.exit_code == 0,
        "#1098: host object picture assignment should exit successfully");
    expect(visual_object_property(picture_path, "one-guid", "PICTURE") == "forms\\customer hero.bmp" &&
            visual_object_property(picture_path, "two-guid", "PICTURE") == "forms\\customer hero.bmp" &&
            visual_object_property(picture_path, "three-guid", "PICTURE") == "forms\\status.bmp" &&
            visual_object_property(picture_path, "other-guid", "PICTURE") == "forms\\other.bmp",
        "#1098: host object picture assignment should assign selected text and preserve unrelated objects");

    const fs::path missing_target_path = temp_root / "missing_target.scx";
    write_synthetic_form_table_for_object_picture(missing_target_path);
    const auto missing_target_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_target_path.string(),
            "--picture-object",
            "--picture", "forms\\customer hero.bmp",
            "--picture-target-unique-id", "one-guid",
            "--picture-target-unique-id", "missing-guid",
            "--json"
        },
        temp_root);
    expect(missing_target_process.exit_code == 4,
        "#1098: missing-target host object picture assignment should return command failure");
    expect(visual_object_property(missing_target_path, "one-guid", "PICTURE") == "forms\\save.bmp" &&
            visual_object_property(missing_target_path, "two-guid", "PICTURE") == "forms\\cancel.bmp",
        "#1098: missing-target host object picture assignment should not mutate the asset");

    const fs::path missing_selector_path = temp_root / "missing_selector.scx";
    write_synthetic_form_table_for_object_picture(missing_selector_path);
    const auto missing_selector_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_selector_path.string(),
            "--picture-object",
            "--picture", "forms\\customer hero.bmp",
            "--json"
        },
        temp_root);
    expect(missing_selector_process.exit_code == 2,
        "#1098: picture-object without target selectors should fail during launch parsing");
    expect(visual_object_property(missing_selector_path, "one-guid", "PICTURE") == "forms\\save.bmp",
        "#1098: picture-object without target selectors should not mutate the asset");

    const fs::path missing_value_path = temp_root / "missing_value.scx";
    write_synthetic_form_table_for_object_picture(missing_value_path);
    const auto missing_value_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_value_path.string(),
            "--picture-object",
            "--picture-target-unique-id", "one-guid",
            "--json"
        },
        temp_root);
    expect(missing_value_process.exit_code == 2,
        "#1098: picture-object without picture value should fail during launch parsing");
    expect(visual_object_property(missing_value_path, "one-guid", "PICTURE") == "forms\\save.bmp",
        "#1098: picture-object without picture value should not mutate the asset");

    const fs::path duplicate_path = temp_root / "duplicate.scx";
    write_synthetic_form_table_for_object_picture(duplicate_path);
    const auto duplicate_process = run_process_capture(
        studio_host_path,
        {
            "--path", duplicate_path.string(),
            "--picture-object",
            "--picture", "forms\\customer hero.bmp",
            "--picture-target-unique-id", "one-guid",
            "--picture-target-object-name", "cmdSave",
            "--json"
        },
        temp_root);
    expect(duplicate_process.exit_code == 4,
        "#1098: duplicate-target host object picture assignment should return command failure");
    expect(visual_object_property(duplicate_path, "one-guid", "PICTURE") == "forms\\save.bmp",
        "#1098: duplicate-target host object picture assignment should not mutate the asset");

    const fs::path ambiguous_path = temp_root / "ambiguous.scx";
    write_synthetic_form_table_for_object_picture(ambiguous_path);
    const auto ambiguous_process = run_process_capture(
        studio_host_path,
        {
            "--path", ambiguous_path.string(),
            "--picture-object",
            "--locked-object",
            "--picture", "forms\\customer hero.bmp",
            "--picture-target-unique-id", "one-guid",
            "--locked", "true",
            "--locked-target-unique-id", "one-guid",
            "--json"
        },
        temp_root);
    expect(ambiguous_process.exit_code == 2,
        "#1098: picture-object plus locked-object requests should fail during launch parsing");
    expect(visual_object_property(ambiguous_path, "one-guid", "PICTURE") == "forms\\save.bmp",
        "#1098: picture-object ambiguity should not mutate the asset");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_assigns_down_picture_by_stable_selectors(const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root = fs::temp_directory_path() / "copperfin_studio_host_down_picture_object_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path down_picture_path = temp_root / "down_picture.scx";
    write_synthetic_form_table_for_object_down_picture(down_picture_path);
    const auto down_picture_process = run_process_capture(
        studio_host_path,
        {
            "--path", down_picture_path.string(),
            "--down-picture-object",
            "--down-picture", "forms\\customer down hero.bmp",
            "--down-picture-target-object-name", "cmdSave",
            "--down-picture-target-unique-id", "two-guid",
            "--json"
        },
        temp_root);
    expect(down_picture_process.exit_code == 0,
        "#1099: host object down-picture assignment should exit successfully");
    expect(visual_object_property(down_picture_path, "one-guid", "DOWNPICTURE") == "forms\\customer down hero.bmp" &&
            visual_object_property(down_picture_path, "two-guid", "DOWNPICTURE") == "forms\\customer down hero.bmp" &&
            visual_object_property(down_picture_path, "three-guid", "DOWNPICTURE") == "forms\\status_down.bmp" &&
            visual_object_property(down_picture_path, "other-guid", "DOWNPICTURE") == "forms\\other_down.bmp",
        "#1099: host object down-picture assignment should assign selected text and preserve unrelated objects");

    const fs::path missing_target_path = temp_root / "missing_target.scx";
    write_synthetic_form_table_for_object_down_picture(missing_target_path);
    const auto missing_target_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_target_path.string(),
            "--down-picture-object",
            "--down-picture", "forms\\customer down hero.bmp",
            "--down-picture-target-unique-id", "one-guid",
            "--down-picture-target-unique-id", "missing-guid",
            "--json"
        },
        temp_root);
    expect(missing_target_process.exit_code == 4,
        "#1099: missing-target host object down-picture assignment should return command failure");
    expect(visual_object_property(missing_target_path, "one-guid", "DOWNPICTURE") == "forms\\save_down.bmp" &&
            visual_object_property(missing_target_path, "two-guid", "DOWNPICTURE") == "forms\\cancel_down.bmp",
        "#1099: missing-target host object down-picture assignment should not mutate the asset");

    const fs::path missing_selector_path = temp_root / "missing_selector.scx";
    write_synthetic_form_table_for_object_down_picture(missing_selector_path);
    const auto missing_selector_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_selector_path.string(),
            "--down-picture-object",
            "--down-picture", "forms\\customer down hero.bmp",
            "--json"
        },
        temp_root);
    expect(missing_selector_process.exit_code == 2,
        "#1099: down-picture-object without target selectors should fail during launch parsing");
    expect(visual_object_property(missing_selector_path, "one-guid", "DOWNPICTURE") == "forms\\save_down.bmp",
        "#1099: down-picture-object without target selectors should not mutate the asset");

    const fs::path missing_value_path = temp_root / "missing_value.scx";
    write_synthetic_form_table_for_object_down_picture(missing_value_path);
    const auto missing_value_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_value_path.string(),
            "--down-picture-object",
            "--down-picture-target-unique-id", "one-guid",
            "--json"
        },
        temp_root);
    expect(missing_value_process.exit_code == 2,
        "#1099: down-picture-object without down-picture value should fail during launch parsing");
    expect(visual_object_property(missing_value_path, "one-guid", "DOWNPICTURE") == "forms\\save_down.bmp",
        "#1099: down-picture-object without down-picture value should not mutate the asset");

    const fs::path duplicate_path = temp_root / "duplicate.scx";
    write_synthetic_form_table_for_object_down_picture(duplicate_path);
    const auto duplicate_process = run_process_capture(
        studio_host_path,
        {
            "--path", duplicate_path.string(),
            "--down-picture-object",
            "--down-picture", "forms\\customer down hero.bmp",
            "--down-picture-target-unique-id", "one-guid",
            "--down-picture-target-object-name", "cmdSave",
            "--json"
        },
        temp_root);
    expect(duplicate_process.exit_code == 4,
        "#1099: duplicate-target host object down-picture assignment should return command failure");
    expect(visual_object_property(duplicate_path, "one-guid", "DOWNPICTURE") == "forms\\save_down.bmp",
        "#1099: duplicate-target host object down-picture assignment should not mutate the asset");

    const fs::path ambiguous_path = temp_root / "ambiguous.scx";
    write_synthetic_form_table_for_object_down_picture(ambiguous_path);
    const auto ambiguous_process = run_process_capture(
        studio_host_path,
        {
            "--path", ambiguous_path.string(),
            "--down-picture-object",
            "--locked-object",
            "--down-picture", "forms\\customer down hero.bmp",
            "--down-picture-target-unique-id", "one-guid",
            "--locked", "true",
            "--locked-target-unique-id", "one-guid",
            "--json"
        },
        temp_root);
    expect(ambiguous_process.exit_code == 2,
        "#1099: down-picture-object plus locked-object requests should fail during launch parsing");
    expect(visual_object_property(ambiguous_path, "one-guid", "DOWNPICTURE") == "forms\\save_down.bmp",
        "#1099: down-picture-object ambiguity should not mutate the asset");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_assigns_disabled_picture_by_stable_selectors(const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root = fs::temp_directory_path() / "copperfin_studio_host_disabled_picture_object_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path disabled_picture_path = temp_root / "disabled_picture.scx";
    write_synthetic_form_table_for_object_disabled_picture(disabled_picture_path);
    const auto disabled_picture_process = run_process_capture(
        studio_host_path,
        {
            "--path", disabled_picture_path.string(),
            "--disabled-picture-object",
            "--disabled-picture", "forms\\customer disabled hero.bmp",
            "--disabled-picture-target-object-name", "cmdSave",
            "--disabled-picture-target-unique-id", "two-guid",
            "--json"
        },
        temp_root);
    expect(disabled_picture_process.exit_code == 0,
        "#1100: host object disabled-picture assignment should exit successfully");
    expect(visual_object_property(disabled_picture_path, "one-guid", "DISABLEDPICTURE") ==
                "forms\\customer disabled hero.bmp" &&
            visual_object_property(disabled_picture_path, "two-guid", "DISABLEDPICTURE") ==
                "forms\\customer disabled hero.bmp" &&
            visual_object_property(disabled_picture_path, "three-guid", "DISABLEDPICTURE") ==
                "forms\\status_disabled.bmp" &&
            visual_object_property(disabled_picture_path, "other-guid", "DISABLEDPICTURE") ==
                "forms\\other_disabled.bmp",
        "#1100: host object disabled-picture assignment should assign selected text and preserve unrelated objects");

    const fs::path missing_target_path = temp_root / "missing_target.scx";
    write_synthetic_form_table_for_object_disabled_picture(missing_target_path);
    const auto missing_target_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_target_path.string(),
            "--disabled-picture-object",
            "--disabled-picture", "forms\\customer disabled hero.bmp",
            "--disabled-picture-target-unique-id", "one-guid",
            "--disabled-picture-target-unique-id", "missing-guid",
            "--json"
        },
        temp_root);
    expect(missing_target_process.exit_code == 4,
        "#1100: missing-target host object disabled-picture assignment should return command failure");
    expect(visual_object_property(missing_target_path, "one-guid", "DISABLEDPICTURE") ==
                "forms\\save_disabled.bmp" &&
            visual_object_property(missing_target_path, "two-guid", "DISABLEDPICTURE") ==
                "forms\\cancel_disabled.bmp",
        "#1100: missing-target host object disabled-picture assignment should not mutate the asset");

    const fs::path missing_selector_path = temp_root / "missing_selector.scx";
    write_synthetic_form_table_for_object_disabled_picture(missing_selector_path);
    const auto missing_selector_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_selector_path.string(),
            "--disabled-picture-object",
            "--disabled-picture", "forms\\customer disabled hero.bmp",
            "--json"
        },
        temp_root);
    expect(missing_selector_process.exit_code == 2,
        "#1100: disabled-picture-object without target selectors should fail during launch parsing");
    expect(visual_object_property(missing_selector_path, "one-guid", "DISABLEDPICTURE") ==
            "forms\\save_disabled.bmp",
        "#1100: disabled-picture-object without target selectors should not mutate the asset");

    const fs::path missing_value_path = temp_root / "missing_value.scx";
    write_synthetic_form_table_for_object_disabled_picture(missing_value_path);
    const auto missing_value_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_value_path.string(),
            "--disabled-picture-object",
            "--disabled-picture-target-unique-id", "one-guid",
            "--json"
        },
        temp_root);
    expect(missing_value_process.exit_code == 2,
        "#1100: disabled-picture-object without disabled-picture value should fail during launch parsing");
    expect(visual_object_property(missing_value_path, "one-guid", "DISABLEDPICTURE") ==
            "forms\\save_disabled.bmp",
        "#1100: disabled-picture-object without disabled-picture value should not mutate the asset");

    const fs::path duplicate_path = temp_root / "duplicate.scx";
    write_synthetic_form_table_for_object_disabled_picture(duplicate_path);
    const auto duplicate_process = run_process_capture(
        studio_host_path,
        {
            "--path", duplicate_path.string(),
            "--disabled-picture-object",
            "--disabled-picture", "forms\\customer disabled hero.bmp",
            "--disabled-picture-target-unique-id", "one-guid",
            "--disabled-picture-target-object-name", "cmdSave",
            "--json"
        },
        temp_root);
    expect(duplicate_process.exit_code == 4,
        "#1100: duplicate-target host object disabled-picture assignment should return command failure");
    expect(visual_object_property(duplicate_path, "one-guid", "DISABLEDPICTURE") ==
            "forms\\save_disabled.bmp",
        "#1100: duplicate-target host object disabled-picture assignment should not mutate the asset");

    const fs::path ambiguous_path = temp_root / "ambiguous.scx";
    write_synthetic_form_table_for_object_disabled_picture(ambiguous_path);
    const auto ambiguous_process = run_process_capture(
        studio_host_path,
        {
            "--path", ambiguous_path.string(),
            "--disabled-picture-object",
            "--locked-object",
            "--disabled-picture", "forms\\customer disabled hero.bmp",
            "--disabled-picture-target-unique-id", "one-guid",
            "--locked", "true",
            "--locked-target-unique-id", "one-guid",
            "--json"
        },
        temp_root);
    expect(ambiguous_process.exit_code == 2,
        "#1100: disabled-picture-object plus locked-object requests should fail during launch parsing");
    expect(visual_object_property(ambiguous_path, "one-guid", "DISABLEDPICTURE") ==
            "forms\\save_disabled.bmp",
        "#1100: disabled-picture-object ambiguity should not mutate the asset");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_assigns_ole_drag_picture_by_stable_selectors(const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root = fs::temp_directory_path() / "copperfin_studio_host_ole_drag_picture_object_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path ole_drag_picture_path = temp_root / "ole_drag_picture.scx";
    write_synthetic_form_table_for_object_ole_drag_picture(ole_drag_picture_path);
    const auto ole_drag_picture_process = run_process_capture(
        studio_host_path,
        {
            "--path", ole_drag_picture_path.string(),
            "--ole-drag-picture-object",
            "--ole-drag-picture", "forms\\customer ole drag hero.bmp",
            "--ole-drag-picture-target-object-name", "cmdSave",
            "--ole-drag-picture-target-unique-id", "two-guid",
            "--json"
        },
        temp_root);
    expect(ole_drag_picture_process.exit_code == 0,
        "#1101: host object OLE drag-picture assignment should exit successfully");
    expect(visual_object_property(ole_drag_picture_path, "one-guid", "OLEDRAGPICTURE") ==
                "forms\\customer ole drag hero.bmp" &&
            visual_object_property(ole_drag_picture_path, "two-guid", "OLEDRAGPICTURE") ==
                "forms\\customer ole drag hero.bmp" &&
            visual_object_property(ole_drag_picture_path, "three-guid", "OLEDRAGPICTURE") ==
                "forms\\status_ole_drag.bmp" &&
            visual_object_property(ole_drag_picture_path, "other-guid", "OLEDRAGPICTURE") ==
                "forms\\other_ole_drag.bmp",
        "#1101: host object OLE drag-picture assignment should assign selected text and preserve unrelated objects");

    const fs::path missing_target_path = temp_root / "missing_target.scx";
    write_synthetic_form_table_for_object_ole_drag_picture(missing_target_path);
    const auto missing_target_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_target_path.string(),
            "--ole-drag-picture-object",
            "--ole-drag-picture", "forms\\customer ole drag hero.bmp",
            "--ole-drag-picture-target-unique-id", "one-guid",
            "--ole-drag-picture-target-unique-id", "missing-guid",
            "--json"
        },
        temp_root);
    expect(missing_target_process.exit_code == 4,
        "#1101: missing-target host object OLE drag-picture assignment should return command failure");
    expect(visual_object_property(missing_target_path, "one-guid", "OLEDRAGPICTURE") ==
                "forms\\save_ole_drag.bmp" &&
            visual_object_property(missing_target_path, "two-guid", "OLEDRAGPICTURE") ==
                "forms\\cancel_ole_drag.bmp",
        "#1101: missing-target host object OLE drag-picture assignment should not mutate the asset");

    const fs::path missing_selector_path = temp_root / "missing_selector.scx";
    write_synthetic_form_table_for_object_ole_drag_picture(missing_selector_path);
    const auto missing_selector_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_selector_path.string(),
            "--ole-drag-picture-object",
            "--ole-drag-picture", "forms\\customer ole drag hero.bmp",
            "--json"
        },
        temp_root);
    expect(missing_selector_process.exit_code == 2,
        "#1101: OLE drag-picture-object without target selectors should fail during launch parsing");
    expect(visual_object_property(missing_selector_path, "one-guid", "OLEDRAGPICTURE") ==
            "forms\\save_ole_drag.bmp",
        "#1101: OLE drag-picture-object without target selectors should not mutate the asset");

    const fs::path missing_value_path = temp_root / "missing_value.scx";
    write_synthetic_form_table_for_object_ole_drag_picture(missing_value_path);
    const auto missing_value_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_value_path.string(),
            "--ole-drag-picture-object",
            "--ole-drag-picture-target-unique-id", "one-guid",
            "--json"
        },
        temp_root);
    expect(missing_value_process.exit_code == 2,
        "#1101: OLE drag-picture-object without OLE drag-picture value should fail during launch parsing");
    expect(visual_object_property(missing_value_path, "one-guid", "OLEDRAGPICTURE") ==
            "forms\\save_ole_drag.bmp",
        "#1101: OLE drag-picture-object without OLE drag-picture value should not mutate the asset");

    const fs::path duplicate_path = temp_root / "duplicate.scx";
    write_synthetic_form_table_for_object_ole_drag_picture(duplicate_path);
    const auto duplicate_process = run_process_capture(
        studio_host_path,
        {
            "--path", duplicate_path.string(),
            "--ole-drag-picture-object",
            "--ole-drag-picture", "forms\\customer ole drag hero.bmp",
            "--ole-drag-picture-target-unique-id", "one-guid",
            "--ole-drag-picture-target-object-name", "cmdSave",
            "--json"
        },
        temp_root);
    expect(duplicate_process.exit_code == 4,
        "#1101: duplicate-target host object OLE drag-picture assignment should return command failure");
    expect(visual_object_property(duplicate_path, "one-guid", "OLEDRAGPICTURE") ==
            "forms\\save_ole_drag.bmp",
        "#1101: duplicate-target host object OLE drag-picture assignment should not mutate the asset");

    const fs::path ambiguous_path = temp_root / "ambiguous.scx";
    write_synthetic_form_table_for_object_ole_drag_picture(ambiguous_path);
    const auto ambiguous_process = run_process_capture(
        studio_host_path,
        {
            "--path", ambiguous_path.string(),
            "--ole-drag-picture-object",
            "--locked-object",
            "--ole-drag-picture", "forms\\customer ole drag hero.bmp",
            "--ole-drag-picture-target-unique-id", "one-guid",
            "--locked", "true",
            "--locked-target-unique-id", "one-guid",
            "--json"
        },
        temp_root);
    expect(ambiguous_process.exit_code == 2,
        "#1101: OLE drag-picture-object plus locked-object requests should fail during launch parsing");
    expect(visual_object_property(ambiguous_path, "one-guid", "OLEDRAGPICTURE") ==
            "forms\\save_ole_drag.bmp",
        "#1101: OLE drag-picture-object ambiguity should not mutate the asset");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_assigns_mouse_icon_by_stable_selectors(const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root = fs::temp_directory_path() / "copperfin_studio_host_mouse_icon_object_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path mouse_icon_path = temp_root / "mouse_icon.scx";
    write_synthetic_form_table_for_object_mouse_icon(mouse_icon_path);
    const auto mouse_icon_process = run_process_capture(
        studio_host_path,
        {
            "--path", mouse_icon_path.string(),
            "--mouse-icon-object",
            "--mouse-icon", "forms\\customer mouse hero.cur",
            "--mouse-icon-target-object-name", "cmdSave",
            "--mouse-icon-target-unique-id", "two-guid",
            "--json"
        },
        temp_root);
    expect(mouse_icon_process.exit_code == 0,
        "#1102: host object mouse-icon assignment should exit successfully");
    expect(visual_object_property(mouse_icon_path, "one-guid", "MOUSEICON") ==
                "forms\\customer mouse hero.cur" &&
            visual_object_property(mouse_icon_path, "two-guid", "MOUSEICON") ==
                "forms\\customer mouse hero.cur" &&
            visual_object_property(mouse_icon_path, "three-guid", "MOUSEICON") ==
                "forms\\status_mouse.cur" &&
            visual_object_property(mouse_icon_path, "other-guid", "MOUSEICON") ==
                "forms\\other_mouse.cur",
        "#1102: host object mouse-icon assignment should assign selected text and preserve unrelated objects");

    const fs::path missing_target_path = temp_root / "missing_target.scx";
    write_synthetic_form_table_for_object_mouse_icon(missing_target_path);
    const auto missing_target_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_target_path.string(),
            "--mouse-icon-object",
            "--mouse-icon", "forms\\customer mouse hero.cur",
            "--mouse-icon-target-unique-id", "one-guid",
            "--mouse-icon-target-unique-id", "missing-guid",
            "--json"
        },
        temp_root);
    expect(missing_target_process.exit_code == 4,
        "#1102: missing-target host object mouse-icon assignment should return command failure");
    expect(visual_object_property(missing_target_path, "one-guid", "MOUSEICON") ==
                "forms\\save_mouse.cur" &&
            visual_object_property(missing_target_path, "two-guid", "MOUSEICON") ==
                "forms\\cancel_mouse.cur",
        "#1102: missing-target host object mouse-icon assignment should not mutate the asset");

    const fs::path missing_selector_path = temp_root / "missing_selector.scx";
    write_synthetic_form_table_for_object_mouse_icon(missing_selector_path);
    const auto missing_selector_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_selector_path.string(),
            "--mouse-icon-object",
            "--mouse-icon", "forms\\customer mouse hero.cur",
            "--json"
        },
        temp_root);
    expect(missing_selector_process.exit_code == 2,
        "#1102: mouse-icon-object without target selectors should fail during launch parsing");
    expect(visual_object_property(missing_selector_path, "one-guid", "MOUSEICON") == "forms\\save_mouse.cur",
        "#1102: mouse-icon-object without target selectors should not mutate the asset");

    const fs::path missing_value_path = temp_root / "missing_value.scx";
    write_synthetic_form_table_for_object_mouse_icon(missing_value_path);
    const auto missing_value_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_value_path.string(),
            "--mouse-icon-object",
            "--mouse-icon-target-unique-id", "one-guid",
            "--json"
        },
        temp_root);
    expect(missing_value_process.exit_code == 2,
        "#1102: mouse-icon-object without mouse-icon value should fail during launch parsing");
    expect(visual_object_property(missing_value_path, "one-guid", "MOUSEICON") == "forms\\save_mouse.cur",
        "#1102: mouse-icon-object without mouse-icon value should not mutate the asset");

    const fs::path duplicate_path = temp_root / "duplicate.scx";
    write_synthetic_form_table_for_object_mouse_icon(duplicate_path);
    const auto duplicate_process = run_process_capture(
        studio_host_path,
        {
            "--path", duplicate_path.string(),
            "--mouse-icon-object",
            "--mouse-icon", "forms\\customer mouse hero.cur",
            "--mouse-icon-target-unique-id", "one-guid",
            "--mouse-icon-target-object-name", "cmdSave",
            "--json"
        },
        temp_root);
    expect(duplicate_process.exit_code == 4,
        "#1102: duplicate-target host object mouse-icon assignment should return command failure");
    expect(visual_object_property(duplicate_path, "one-guid", "MOUSEICON") == "forms\\save_mouse.cur",
        "#1102: duplicate-target host object mouse-icon assignment should not mutate the asset");

    const fs::path ambiguous_path = temp_root / "ambiguous.scx";
    write_synthetic_form_table_for_object_mouse_icon(ambiguous_path);
    const auto ambiguous_process = run_process_capture(
        studio_host_path,
        {
            "--path", ambiguous_path.string(),
            "--mouse-icon-object",
            "--locked-object",
            "--mouse-icon", "forms\\customer mouse hero.cur",
            "--mouse-icon-target-unique-id", "one-guid",
            "--locked", "true",
            "--locked-target-unique-id", "one-guid",
            "--json"
        },
        temp_root);
    expect(ambiguous_process.exit_code == 2,
        "#1102: mouse-icon-object plus locked-object requests should fail during launch parsing");
    expect(visual_object_property(ambiguous_path, "one-guid", "MOUSEICON") == "forms\\save_mouse.cur",
        "#1102: mouse-icon-object ambiguity should not mutate the asset");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_assigns_drag_icon_by_stable_selectors(const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root = fs::temp_directory_path() / "copperfin_studio_host_drag_icon_object_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path drag_icon_path = temp_root / "drag_icon.scx";
    write_synthetic_form_table_for_object_drag_icon(drag_icon_path);
    const auto drag_icon_process = run_process_capture(
        studio_host_path,
        {
            "--path", drag_icon_path.string(),
            "--drag-icon-object",
            "--drag-icon", "forms\\customer drag hero.cur",
            "--drag-icon-target-object-name", "cmdSave",
            "--drag-icon-target-unique-id", "two-guid",
            "--json"
        },
        temp_root);
    expect(drag_icon_process.exit_code == 0,
        "#1103: host object drag-icon assignment should exit successfully");
    expect(visual_object_property(drag_icon_path, "one-guid", "DRAGICON") == "forms\\customer drag hero.cur" &&
            visual_object_property(drag_icon_path, "two-guid", "DRAGICON") == "forms\\customer drag hero.cur" &&
            visual_object_property(drag_icon_path, "three-guid", "DRAGICON") == "forms\\status_drag.cur" &&
            visual_object_property(drag_icon_path, "other-guid", "DRAGICON") == "forms\\other_drag.cur",
        "#1103: host object drag-icon assignment should assign selected text and preserve unrelated objects");

    const fs::path missing_target_path = temp_root / "missing_target.scx";
    write_synthetic_form_table_for_object_drag_icon(missing_target_path);
    const auto missing_target_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_target_path.string(),
            "--drag-icon-object",
            "--drag-icon", "forms\\customer drag hero.cur",
            "--drag-icon-target-unique-id", "one-guid",
            "--drag-icon-target-unique-id", "missing-guid",
            "--json"
        },
        temp_root);
    expect(missing_target_process.exit_code == 4,
        "#1103: missing-target host object drag-icon assignment should return command failure");
    expect(visual_object_property(missing_target_path, "one-guid", "DRAGICON") == "forms\\save_drag.cur" &&
            visual_object_property(missing_target_path, "two-guid", "DRAGICON") == "forms\\cancel_drag.cur",
        "#1103: missing-target host object drag-icon assignment should not mutate the asset");

    const fs::path missing_selector_path = temp_root / "missing_selector.scx";
    write_synthetic_form_table_for_object_drag_icon(missing_selector_path);
    const auto missing_selector_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_selector_path.string(),
            "--drag-icon-object",
            "--drag-icon", "forms\\customer drag hero.cur",
            "--json"
        },
        temp_root);
    expect(missing_selector_process.exit_code == 2,
        "#1103: drag-icon-object without target selectors should fail during launch parsing");
    expect(visual_object_property(missing_selector_path, "one-guid", "DRAGICON") == "forms\\save_drag.cur",
        "#1103: drag-icon-object without target selectors should not mutate the asset");

    const fs::path missing_value_path = temp_root / "missing_value.scx";
    write_synthetic_form_table_for_object_drag_icon(missing_value_path);
    const auto missing_value_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_value_path.string(),
            "--drag-icon-object",
            "--drag-icon-target-unique-id", "one-guid",
            "--json"
        },
        temp_root);
    expect(missing_value_process.exit_code == 2,
        "#1103: drag-icon-object without drag-icon value should fail during launch parsing");
    expect(visual_object_property(missing_value_path, "one-guid", "DRAGICON") == "forms\\save_drag.cur",
        "#1103: drag-icon-object without drag-icon value should not mutate the asset");

    const fs::path duplicate_path = temp_root / "duplicate.scx";
    write_synthetic_form_table_for_object_drag_icon(duplicate_path);
    const auto duplicate_process = run_process_capture(
        studio_host_path,
        {
            "--path", duplicate_path.string(),
            "--drag-icon-object",
            "--drag-icon", "forms\\customer drag hero.cur",
            "--drag-icon-target-unique-id", "one-guid",
            "--drag-icon-target-object-name", "cmdSave",
            "--json"
        },
        temp_root);
    expect(duplicate_process.exit_code == 4,
        "#1103: duplicate-target host object drag-icon assignment should return command failure");
    expect(visual_object_property(duplicate_path, "one-guid", "DRAGICON") == "forms\\save_drag.cur",
        "#1103: duplicate-target host object drag-icon assignment should not mutate the asset");

    const fs::path ambiguous_path = temp_root / "ambiguous.scx";
    write_synthetic_form_table_for_object_drag_icon(ambiguous_path);
    const auto ambiguous_process = run_process_capture(
        studio_host_path,
        {
            "--path", ambiguous_path.string(),
            "--drag-icon-object",
            "--locked-object",
            "--drag-icon", "forms\\customer drag hero.cur",
            "--drag-icon-target-unique-id", "one-guid",
            "--locked", "true",
            "--locked-target-unique-id", "one-guid",
            "--json"
        },
        temp_root);
    expect(ambiguous_process.exit_code == 2,
        "#1103: drag-icon-object plus locked-object requests should fail during launch parsing");
    expect(visual_object_property(ambiguous_path, "one-guid", "DRAGICON") == "forms\\save_drag.cur",
        "#1103: drag-icon-object ambiguity should not mutate the asset");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_assigns_picture_margin_by_stable_selectors(const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root = fs::temp_directory_path() / "copperfin_studio_host_picture_margin_object_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path picture_margin_path = temp_root / "picture_margin.scx";
    write_synthetic_form_table_for_object_picture_margin(picture_margin_path);
    const auto picture_margin_process = run_process_capture(
        studio_host_path,
        {
            "--path", picture_margin_path.string(),
            "--picture-margin-object",
            "--picture-margin", "2",
            "--picture-margin-target-object-name", "frmCustomer",
            "--picture-margin-target-unique-id", "two-guid",
            "--json"
        },
        temp_root);
    expect(picture_margin_process.exit_code == 0,
        "#1172: host object picture-margin assignment should exit successfully");
    expect(visual_object_property(picture_margin_path, "one-guid", "PICTUREMARGIN") == "2" &&
            visual_object_property(picture_margin_path, "two-guid", "PICTUREMARGIN") == "2" &&
            visual_object_property(picture_margin_path, "three-guid", "PICTUREMARGIN") == "1" &&
            visual_object_property(picture_margin_path, "other-guid", "PICTUREMARGIN") == "1",
        "#1172: host object picture-margin assignment should assign selected numeric value and preserve unrelated objects");

    const fs::path missing_target_path = temp_root / "missing_target.scx";
    write_synthetic_form_table_for_object_picture_margin(missing_target_path);
    const auto missing_target_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_target_path.string(),
            "--picture-margin-object",
            "--picture-margin", "2",
            "--picture-margin-target-unique-id", "one-guid",
            "--picture-margin-target-unique-id", "missing-guid",
            "--json"
        },
        temp_root);
    expect(missing_target_process.exit_code == 4,
        "#1172: missing-target host object picture-margin assignment should return command failure");
    expect(visual_object_property(missing_target_path, "one-guid", "PICTUREMARGIN") == "0" &&
            visual_object_property(missing_target_path, "two-guid", "PICTUREMARGIN") == "0",
        "#1172: missing-target host object picture-margin assignment should not mutate the asset");

    const fs::path missing_selector_path = temp_root / "missing_selector.scx";
    write_synthetic_form_table_for_object_picture_margin(missing_selector_path);
    const auto missing_selector_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_selector_path.string(),
            "--picture-margin-object",
            "--picture-margin", "2",
            "--json"
        },
        temp_root);
    expect(missing_selector_process.exit_code == 2,
        "#1172: picture-margin-object without target selectors should fail during launch parsing");
    expect(visual_object_property(missing_selector_path, "one-guid", "PICTUREMARGIN") == "0",
        "#1172: picture-margin-object without target selectors should not mutate the asset");

    const fs::path missing_value_path = temp_root / "missing_value.scx";
    write_synthetic_form_table_for_object_picture_margin(missing_value_path);
    const auto missing_value_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_value_path.string(),
            "--picture-margin-object",
            "--picture-margin-target-unique-id", "one-guid",
            "--json"
        },
        temp_root);
    expect(missing_value_process.exit_code == 2,
        "#1172: picture-margin-object without picture-margin value should fail during launch parsing");
    expect(visual_object_property(missing_value_path, "one-guid", "PICTUREMARGIN") == "0",
        "#1172: picture-margin-object without picture-margin value should not mutate the asset");

    const fs::path negative_value_path = temp_root / "negative_value.scx";
    write_synthetic_form_table_for_object_picture_margin(negative_value_path);
    const auto negative_value_process = run_process_capture(
        studio_host_path,
        {
            "--path", negative_value_path.string(),
            "--picture-margin-object",
            "--picture-margin", "-1",
            "--picture-margin-target-unique-id", "one-guid",
            "--json"
        },
        temp_root);
    expect(negative_value_process.exit_code == 2,
        "#1172: negative picture-margin values should fail during launch parsing");
    expect(visual_object_property(negative_value_path, "one-guid", "PICTUREMARGIN") == "0",
        "#1172: negative picture-margin values should not mutate the asset");

    const fs::path duplicate_path = temp_root / "duplicate.scx";
    write_synthetic_form_table_for_object_picture_margin(duplicate_path);
    const auto duplicate_process = run_process_capture(
        studio_host_path,
        {
            "--path", duplicate_path.string(),
            "--picture-margin-object",
            "--picture-margin", "2",
            "--picture-margin-target-unique-id", "one-guid",
            "--picture-margin-target-object-name", "frmCustomer",
            "--json"
        },
        temp_root);
    expect(duplicate_process.exit_code == 4,
        "#1172: duplicate-target host object picture-margin assignment should return command failure");
    expect(visual_object_property(duplicate_path, "one-guid", "PICTUREMARGIN") == "0",
        "#1172: duplicate-target host object picture-margin assignment should not mutate the asset");

    const fs::path ambiguous_path = temp_root / "ambiguous.scx";
    write_synthetic_form_table_for_object_picture_margin(ambiguous_path);
    const auto ambiguous_process = run_process_capture(
        studio_host_path,
        {
            "--path", ambiguous_path.string(),
            "--picture-margin-object",
            "--allow-output-object",
            "--picture-margin", "2",
            "--picture-margin-target-unique-id", "one-guid",
            "--allow-output", "false",
            "--allow-output-target-unique-id", "one-guid",
            "--json"
        },
        temp_root);
    expect(ambiguous_process.exit_code == 2,
        "#1172: picture-margin-object plus allow-output-object requests should fail during launch parsing");
    expect(visual_object_property(ambiguous_path, "one-guid", "PICTUREMARGIN") == "0",
        "#1172: picture-margin-object ambiguity should not mutate the asset");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_assigns_picture_position_by_stable_selectors(const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root = fs::temp_directory_path() / "copperfin_studio_host_picture_position_object_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path picture_position_path = temp_root / "picture_position.scx";
    write_synthetic_form_table_for_object_picture_position(picture_position_path);
    const auto picture_position_process = run_process_capture(
        studio_host_path,
        {
            "--path", picture_position_path.string(),
            "--picture-position-object",
            "--picture-position", "2",
            "--picture-position-target-object-name", "frmCustomer",
            "--picture-position-target-unique-id", "two-guid",
            "--json"
        },
        temp_root);
    expect(picture_position_process.exit_code == 0,
        "#1173: host object picture-position assignment should exit successfully");
    expect(visual_object_property(picture_position_path, "one-guid", "PICTUREPOSITION") == "2" &&
            visual_object_property(picture_position_path, "two-guid", "PICTUREPOSITION") == "2" &&
            visual_object_property(picture_position_path, "three-guid", "PICTUREPOSITION") == "1" &&
            visual_object_property(picture_position_path, "other-guid", "PICTUREPOSITION") == "1",
        "#1173: host object picture-position assignment should assign selected numeric value and preserve unrelated objects");

    const fs::path missing_target_path = temp_root / "missing_target.scx";
    write_synthetic_form_table_for_object_picture_position(missing_target_path);
    const auto missing_target_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_target_path.string(),
            "--picture-position-object",
            "--picture-position", "2",
            "--picture-position-target-unique-id", "one-guid",
            "--picture-position-target-unique-id", "missing-guid",
            "--json"
        },
        temp_root);
    expect(missing_target_process.exit_code == 4,
        "#1173: missing-target host object picture-position assignment should return command failure");
    expect(visual_object_property(missing_target_path, "one-guid", "PICTUREPOSITION") == "0" &&
            visual_object_property(missing_target_path, "two-guid", "PICTUREPOSITION") == "0",
        "#1173: missing-target host object picture-position assignment should not mutate the asset");

    const fs::path missing_selector_path = temp_root / "missing_selector.scx";
    write_synthetic_form_table_for_object_picture_position(missing_selector_path);
    const auto missing_selector_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_selector_path.string(),
            "--picture-position-object",
            "--picture-position", "2",
            "--json"
        },
        temp_root);
    expect(missing_selector_process.exit_code == 2,
        "#1173: picture-position-object without target selectors should fail during launch parsing");
    expect(visual_object_property(missing_selector_path, "one-guid", "PICTUREPOSITION") == "0",
        "#1173: picture-position-object without target selectors should not mutate the asset");

    const fs::path missing_value_path = temp_root / "missing_value.scx";
    write_synthetic_form_table_for_object_picture_position(missing_value_path);
    const auto missing_value_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_value_path.string(),
            "--picture-position-object",
            "--picture-position-target-unique-id", "one-guid",
            "--json"
        },
        temp_root);
    expect(missing_value_process.exit_code == 2,
        "#1173: picture-position-object without picture-position value should fail during launch parsing");
    expect(visual_object_property(missing_value_path, "one-guid", "PICTUREPOSITION") == "0",
        "#1173: picture-position-object without picture-position value should not mutate the asset");

    const fs::path negative_value_path = temp_root / "negative_value.scx";
    write_synthetic_form_table_for_object_picture_position(negative_value_path);
    const auto negative_value_process = run_process_capture(
        studio_host_path,
        {
            "--path", negative_value_path.string(),
            "--picture-position-object",
            "--picture-position", "-1",
            "--picture-position-target-unique-id", "one-guid",
            "--json"
        },
        temp_root);
    expect(negative_value_process.exit_code == 2,
        "#1173: negative picture-position values should fail during launch parsing");
    expect(visual_object_property(negative_value_path, "one-guid", "PICTUREPOSITION") == "0",
        "#1173: negative picture-position values should not mutate the asset");

    const fs::path duplicate_path = temp_root / "duplicate.scx";
    write_synthetic_form_table_for_object_picture_position(duplicate_path);
    const auto duplicate_process = run_process_capture(
        studio_host_path,
        {
            "--path", duplicate_path.string(),
            "--picture-position-object",
            "--picture-position", "2",
            "--picture-position-target-unique-id", "one-guid",
            "--picture-position-target-object-name", "frmCustomer",
            "--json"
        },
        temp_root);
    expect(duplicate_process.exit_code == 4,
        "#1173: duplicate-target host object picture-position assignment should return command failure");
    expect(visual_object_property(duplicate_path, "one-guid", "PICTUREPOSITION") == "0",
        "#1173: duplicate-target host object picture-position assignment should not mutate the asset");

    const fs::path ambiguous_path = temp_root / "ambiguous.scx";
    write_synthetic_form_table_for_object_picture_position(ambiguous_path);
    const auto ambiguous_process = run_process_capture(
        studio_host_path,
        {
            "--path", ambiguous_path.string(),
            "--picture-position-object",
            "--allow-output-object",
            "--picture-position", "2",
            "--picture-position-target-unique-id", "one-guid",
            "--allow-output", "false",
            "--allow-output-target-unique-id", "one-guid",
            "--json"
        },
        temp_root);
    expect(ambiguous_process.exit_code == 2,
        "#1173: picture-position-object plus allow-output-object requests should fail during launch parsing");
    expect(visual_object_property(ambiguous_path, "one-guid", "PICTUREPOSITION") == "0",
        "#1173: picture-position-object ambiguity should not mutate the asset");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_assigns_picture_spacing_by_stable_selectors(const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root = fs::temp_directory_path() / "copperfin_studio_host_picture_spacing_object_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path picture_spacing_path = temp_root / "picture_spacing.scx";
    write_synthetic_form_table_for_object_picture_spacing(picture_spacing_path);
    const auto picture_spacing_process = run_process_capture(
        studio_host_path,
        {
            "--path", picture_spacing_path.string(),
            "--picture-spacing-object",
            "--picture-spacing", "2",
            "--picture-spacing-target-object-name", "frmCustomer",
            "--picture-spacing-target-unique-id", "two-guid",
            "--json"
        },
        temp_root);
    expect(picture_spacing_process.exit_code == 0,
        "#1174: host object picture-spacing assignment should exit successfully");
    expect(visual_object_property(picture_spacing_path, "one-guid", "PICTURESPACING") == "2" &&
            visual_object_property(picture_spacing_path, "two-guid", "PICTURESPACING") == "2" &&
            visual_object_property(picture_spacing_path, "three-guid", "PICTURESPACING") == "1" &&
            visual_object_property(picture_spacing_path, "other-guid", "PICTURESPACING") == "1",
        "#1174: host object picture-spacing assignment should assign selected numeric value and preserve unrelated objects");

    const fs::path missing_target_path = temp_root / "missing_target.scx";
    write_synthetic_form_table_for_object_picture_spacing(missing_target_path);
    const auto missing_target_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_target_path.string(),
            "--picture-spacing-object",
            "--picture-spacing", "2",
            "--picture-spacing-target-unique-id", "one-guid",
            "--picture-spacing-target-unique-id", "missing-guid",
            "--json"
        },
        temp_root);
    expect(missing_target_process.exit_code == 4,
        "#1174: missing-target host object picture-spacing assignment should return command failure");
    expect(visual_object_property(missing_target_path, "one-guid", "PICTURESPACING") == "0" &&
            visual_object_property(missing_target_path, "two-guid", "PICTURESPACING") == "0",
        "#1174: missing-target host object picture-spacing assignment should not mutate the asset");

    const fs::path missing_selector_path = temp_root / "missing_selector.scx";
    write_synthetic_form_table_for_object_picture_spacing(missing_selector_path);
    const auto missing_selector_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_selector_path.string(),
            "--picture-spacing-object",
            "--picture-spacing", "2",
            "--json"
        },
        temp_root);
    expect(missing_selector_process.exit_code == 2,
        "#1174: picture-spacing-object without target selectors should fail during launch parsing");
    expect(visual_object_property(missing_selector_path, "one-guid", "PICTURESPACING") == "0",
        "#1174: picture-spacing-object without target selectors should not mutate the asset");

    const fs::path missing_value_path = temp_root / "missing_value.scx";
    write_synthetic_form_table_for_object_picture_spacing(missing_value_path);
    const auto missing_value_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_value_path.string(),
            "--picture-spacing-object",
            "--picture-spacing-target-unique-id", "one-guid",
            "--json"
        },
        temp_root);
    expect(missing_value_process.exit_code == 2,
        "#1174: picture-spacing-object without picture-spacing value should fail during launch parsing");
    expect(visual_object_property(missing_value_path, "one-guid", "PICTURESPACING") == "0",
        "#1174: picture-spacing-object without picture-spacing value should not mutate the asset");

    const fs::path negative_value_path = temp_root / "negative_value.scx";
    write_synthetic_form_table_for_object_picture_spacing(negative_value_path);
    const auto negative_value_process = run_process_capture(
        studio_host_path,
        {
            "--path", negative_value_path.string(),
            "--picture-spacing-object",
            "--picture-spacing", "-1",
            "--picture-spacing-target-unique-id", "one-guid",
            "--json"
        },
        temp_root);
    expect(negative_value_process.exit_code == 2,
        "#1174: negative picture-spacing values should fail during launch parsing");
    expect(visual_object_property(negative_value_path, "one-guid", "PICTURESPACING") == "0",
        "#1174: negative picture-spacing values should not mutate the asset");

    const fs::path duplicate_path = temp_root / "duplicate.scx";
    write_synthetic_form_table_for_object_picture_spacing(duplicate_path);
    const auto duplicate_process = run_process_capture(
        studio_host_path,
        {
            "--path", duplicate_path.string(),
            "--picture-spacing-object",
            "--picture-spacing", "2",
            "--picture-spacing-target-unique-id", "one-guid",
            "--picture-spacing-target-object-name", "frmCustomer",
            "--json"
        },
        temp_root);
    expect(duplicate_process.exit_code == 4,
        "#1174: duplicate-target host object picture-spacing assignment should return command failure");
    expect(visual_object_property(duplicate_path, "one-guid", "PICTURESPACING") == "0",
        "#1174: duplicate-target host object picture-spacing assignment should not mutate the asset");

    const fs::path ambiguous_path = temp_root / "ambiguous.scx";
    write_synthetic_form_table_for_object_picture_spacing(ambiguous_path);
    const auto ambiguous_process = run_process_capture(
        studio_host_path,
        {
            "--path", ambiguous_path.string(),
            "--picture-spacing-object",
            "--allow-output-object",
            "--picture-spacing", "2",
            "--picture-spacing-target-unique-id", "one-guid",
            "--allow-output", "false",
            "--allow-output-target-unique-id", "one-guid",
            "--json"
        },
        temp_root);
    expect(ambiguous_process.exit_code == 2,
        "#1174: picture-spacing-object plus allow-output-object requests should fail during launch parsing");
    expect(visual_object_property(ambiguous_path, "one-guid", "PICTURESPACING") == "0",
        "#1174: picture-spacing-object ambiguity should not mutate the asset");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_assigns_picture_selection_display_by_stable_selectors(const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root = fs::temp_directory_path() / "copperfin_studio_host_picture_selection_display_object_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path picture_selection_display_path = temp_root / "picture_selection_display.scx";
    write_synthetic_form_table_for_object_picture_selection_display(picture_selection_display_path);
    const auto picture_selection_display_process = run_process_capture(
        studio_host_path,
        {
            "--path", picture_selection_display_path.string(),
            "--picture-selection-display-object",
            "--picture-selection-display", "2",
            "--picture-selection-display-target-object-name", "frmCustomer",
            "--picture-selection-display-target-unique-id", "two-guid",
            "--json"
        },
        temp_root);
    expect(picture_selection_display_process.exit_code == 0,
        "#1175: host object picture-selection-display assignment should exit successfully");
    expect(visual_object_property(picture_selection_display_path, "one-guid", "PICTURESELECTIONDISPLAY") == "2" &&
            visual_object_property(picture_selection_display_path, "two-guid", "PICTURESELECTIONDISPLAY") == "2" &&
            visual_object_property(picture_selection_display_path, "three-guid", "PICTURESELECTIONDISPLAY") == "1" &&
            visual_object_property(picture_selection_display_path, "other-guid", "PICTURESELECTIONDISPLAY") == "1",
        "#1175: host object picture-selection-display assignment should assign selected numeric value and preserve unrelated objects");

    const fs::path missing_target_path = temp_root / "missing_target.scx";
    write_synthetic_form_table_for_object_picture_selection_display(missing_target_path);
    const auto missing_target_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_target_path.string(),
            "--picture-selection-display-object",
            "--picture-selection-display", "2",
            "--picture-selection-display-target-unique-id", "one-guid",
            "--picture-selection-display-target-unique-id", "missing-guid",
            "--json"
        },
        temp_root);
    expect(missing_target_process.exit_code == 4,
        "#1175: missing-target host object picture-selection-display assignment should return command failure");
    expect(visual_object_property(missing_target_path, "one-guid", "PICTURESELECTIONDISPLAY") == "0" &&
            visual_object_property(missing_target_path, "two-guid", "PICTURESELECTIONDISPLAY") == "0",
        "#1175: missing-target host object picture-selection-display assignment should not mutate the asset");

    const fs::path missing_selector_path = temp_root / "missing_selector.scx";
    write_synthetic_form_table_for_object_picture_selection_display(missing_selector_path);
    const auto missing_selector_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_selector_path.string(),
            "--picture-selection-display-object",
            "--picture-selection-display", "2",
            "--json"
        },
        temp_root);
    expect(missing_selector_process.exit_code == 2,
        "#1175: picture-selection-display-object without target selectors should fail during launch parsing");
    expect(visual_object_property(missing_selector_path, "one-guid", "PICTURESELECTIONDISPLAY") == "0",
        "#1175: picture-selection-display-object without target selectors should not mutate the asset");

    const fs::path missing_value_path = temp_root / "missing_value.scx";
    write_synthetic_form_table_for_object_picture_selection_display(missing_value_path);
    const auto missing_value_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_value_path.string(),
            "--picture-selection-display-object",
            "--picture-selection-display-target-unique-id", "one-guid",
            "--json"
        },
        temp_root);
    expect(missing_value_process.exit_code == 2,
        "#1175: picture-selection-display-object without picture-selection-display value should fail during launch parsing");
    expect(visual_object_property(missing_value_path, "one-guid", "PICTURESELECTIONDISPLAY") == "0",
        "#1175: picture-selection-display-object without picture-selection-display value should not mutate the asset");

    const fs::path negative_value_path = temp_root / "negative_value.scx";
    write_synthetic_form_table_for_object_picture_selection_display(negative_value_path);
    const auto negative_value_process = run_process_capture(
        studio_host_path,
        {
            "--path", negative_value_path.string(),
            "--picture-selection-display-object",
            "--picture-selection-display", "-1",
            "--picture-selection-display-target-unique-id", "one-guid",
            "--json"
        },
        temp_root);
    expect(negative_value_process.exit_code == 2,
        "#1175: negative picture-selection-display values should fail during launch parsing");
    expect(visual_object_property(negative_value_path, "one-guid", "PICTURESELECTIONDISPLAY") == "0",
        "#1175: negative picture-selection-display values should not mutate the asset");

    const fs::path duplicate_path = temp_root / "duplicate.scx";
    write_synthetic_form_table_for_object_picture_selection_display(duplicate_path);
    const auto duplicate_process = run_process_capture(
        studio_host_path,
        {
            "--path", duplicate_path.string(),
            "--picture-selection-display-object",
            "--picture-selection-display", "2",
            "--picture-selection-display-target-unique-id", "one-guid",
            "--picture-selection-display-target-object-name", "frmCustomer",
            "--json"
        },
        temp_root);
    expect(duplicate_process.exit_code == 4,
        "#1175: duplicate-target host object picture-selection-display assignment should return command failure");
    expect(visual_object_property(duplicate_path, "one-guid", "PICTURESELECTIONDISPLAY") == "0",
        "#1175: duplicate-target host object picture-selection-display assignment should not mutate the asset");

    const fs::path ambiguous_path = temp_root / "ambiguous.scx";
    write_synthetic_form_table_for_object_picture_selection_display(ambiguous_path);
    const auto ambiguous_process = run_process_capture(
        studio_host_path,
        {
            "--path", ambiguous_path.string(),
            "--picture-selection-display-object",
            "--allow-output-object",
            "--picture-selection-display", "2",
            "--picture-selection-display-target-unique-id", "one-guid",
            "--allow-output", "false",
            "--allow-output-target-unique-id", "one-guid",
            "--json"
        },
        temp_root);
    expect(ambiguous_process.exit_code == 2,
        "#1175: picture-selection-display-object plus allow-output-object requests should fail during launch parsing");
    expect(visual_object_property(ambiguous_path, "one-guid", "PICTURESELECTIONDISPLAY") == "0",
        "#1175: picture-selection-display-object ambiguity should not mutate the asset");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

}  // namespace cf_test_studio_host_json
