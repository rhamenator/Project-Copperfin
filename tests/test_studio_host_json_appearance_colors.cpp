// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only

#include "test_studio_host_json_support.h"

namespace cf_test_studio_host_json {
void write_synthetic_form_table_for_object_selected_back_color(const std::filesystem::path& form_path) {
    const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
        {.name = "OBJNAME", .type = 'C', .length = 24U},
        {.name = "NAME", .type = 'C', .length = 24U},
        {.name = "UNIQUEID", .type = 'C', .length = 32U},
        {.name = "SELECTEDBA", .type = 'C', .length = 16U}
    };
    const std::vector<std::vector<std::string>> records{
        {"lstCustomers", "lstCustomers", "one-guid", "16777215"},
        {"lstOrders", "lstOrders", "two-guid", "12632256"},
        {"lblStatus", "lblStatus", "three-guid", "255"},
        {"lstOther", "lstOther", "other-guid", "65280"}
    };

    const auto create_result = copperfin::vfp::create_dbf_table_file(form_path.string(), fields, records);
    expect(create_result.ok, "#1056: synthetic SCX table for object selected back color should be created");
}

void write_synthetic_form_table_for_object_selected_fore_color(const std::filesystem::path& form_path) {
    const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
        {.name = "OBJNAME", .type = 'C', .length = 24U},
        {.name = "NAME", .type = 'C', .length = 24U},
        {.name = "UNIQUEID", .type = 'C', .length = 32U},
        {.name = "SELECTEDFO", .type = 'C', .length = 16U}
    };
    const std::vector<std::vector<std::string>> records{
        {"lstCustomers", "lstCustomers", "one-guid", "0"},
        {"lstOrders", "lstOrders", "two-guid", "255"},
        {"lblStatus", "lblStatus", "three-guid", "16777215"},
        {"lstOther", "lstOther", "other-guid", "65280"}
    };

    const auto create_result = copperfin::vfp::create_dbf_table_file(form_path.string(), fields, records);
    expect(create_result.ok, "#1057: synthetic SCX table for object selected fore color should be created");
}

void write_synthetic_form_table_for_object_selected_item_back_color(const std::filesystem::path& form_path) {
    const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
        {.name = "OBJNAME", .type = 'C', .length = 24U},
        {.name = "NAME", .type = 'C', .length = 24U},
        {.name = "UNIQUEID", .type = 'C', .length = 32U},
        {.name = "SELECTEDIT", .type = 'C', .length = 16U}
    };
    const std::vector<std::vector<std::string>> records{
        {"lstCustomers", "lstCustomers", "one-guid", "16777215"},
        {"lstOrders", "lstOrders", "two-guid", "12632256"},
        {"lblStatus", "lblStatus", "three-guid", "255"},
        {"lstOther", "lstOther", "other-guid", "65280"}
    };

    const auto create_result = copperfin::vfp::create_dbf_table_file(form_path.string(), fields, records);
    expect(create_result.ok, "#1058: synthetic SCX table for object selected item back color should be created");
}

void write_synthetic_form_table_for_object_selected_item_fore_color(const std::filesystem::path& form_path) {
    const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
        {.name = "OBJNAME", .type = 'C', .length = 24U},
        {.name = "NAME", .type = 'C', .length = 24U},
        {.name = "UNIQUEID", .type = 'C', .length = 32U},
        {.name = "SELECTEDIT", .type = 'C', .length = 16U}
    };
    const std::vector<std::vector<std::string>> records{
        {"lstCustomers", "lstCustomers", "one-guid", "0"},
        {"lstOrders", "lstOrders", "two-guid", "16777215"},
        {"lblStatus", "lblStatus", "three-guid", "255"},
        {"lstOther", "lstOther", "other-guid", "65280"}
    };

    const auto create_result = copperfin::vfp::create_dbf_table_file(form_path.string(), fields, records);
    expect(create_result.ok, "#1059: synthetic SCX table for object selected item fore color should be created");
}

void write_synthetic_form_table_for_object_disabled_item_back_color(const std::filesystem::path& form_path) {
    const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
        {.name = "OBJNAME", .type = 'C', .length = 24U},
        {.name = "NAME", .type = 'C', .length = 24U},
        {.name = "UNIQUEID", .type = 'C', .length = 32U},
        {.name = "DISABLEDIT", .type = 'C', .length = 16U}
    };
    const std::vector<std::vector<std::string>> records{
        {"lstCustomers", "lstCustomers", "one-guid", "16777215"},
        {"lstOrders", "lstOrders", "two-guid", "12632256"},
        {"lblStatus", "lblStatus", "three-guid", "255"},
        {"lstOther", "lstOther", "other-guid", "65280"}
    };

    const auto create_result = copperfin::vfp::create_dbf_table_file(form_path.string(), fields, records);
    expect(create_result.ok, "#1060: synthetic SCX table for object disabled item back color should be created");
}

void write_synthetic_form_table_for_object_disabled_item_fore_color(const std::filesystem::path& form_path) {
    const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
        {.name = "OBJNAME", .type = 'C', .length = 24U},
        {.name = "NAME", .type = 'C', .length = 24U},
        {.name = "UNIQUEID", .type = 'C', .length = 32U},
        {.name = "DISABLEDIT", .type = 'C', .length = 16U}
    };
    const std::vector<std::vector<std::string>> records{
        {"lstCustomers", "lstCustomers", "one-guid", "0"},
        {"lstOrders", "lstOrders", "two-guid", "16777215"},
        {"lblStatus", "lblStatus", "three-guid", "255"},
        {"lstOther", "lstOther", "other-guid", "65280"}
    };

    const auto create_result = copperfin::vfp::create_dbf_table_file(form_path.string(), fields, records);
    expect(create_result.ok, "#1061: synthetic SCX table for object disabled item fore color should be created");
}

void write_synthetic_form_table_for_object_item_back_color(const std::filesystem::path& form_path) {
    const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
        {.name = "OBJNAME", .type = 'C', .length = 24U},
        {.name = "NAME", .type = 'C', .length = 24U},
        {.name = "UNIQUEID", .type = 'C', .length = 32U},
        {.name = "ITEMBACKCO", .type = 'C', .length = 16U}
    };
    const std::vector<std::vector<std::string>> records{
        {"lstCustomers", "lstCustomers", "one-guid", "16777215"},
        {"lstOrders", "lstOrders", "two-guid", "12632256"},
        {"lblStatus", "lblStatus", "three-guid", "255"},
        {"lstOther", "lstOther", "other-guid", "65280"}
    };

    const auto create_result = copperfin::vfp::create_dbf_table_file(form_path.string(), fields, records);
    expect(create_result.ok, "#1062: synthetic SCX table for object item back color should be created");
}

void write_synthetic_form_table_for_object_item_fore_color(const std::filesystem::path& form_path) {
    const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
        {.name = "OBJNAME", .type = 'C', .length = 24U},
        {.name = "NAME", .type = 'C', .length = 24U},
        {.name = "UNIQUEID", .type = 'C', .length = 32U},
        {.name = "ITEMFORECO", .type = 'C', .length = 16U}
    };
    const std::vector<std::vector<std::string>> records{
        {"lstCustomers", "lstCustomers", "one-guid", "0"},
        {"lstOrders", "lstOrders", "two-guid", "16777215"},
        {"lblStatus", "lblStatus", "three-guid", "255"},
        {"lstOther", "lstOther", "other-guid", "65280"}
    };

    const auto create_result = copperfin::vfp::create_dbf_table_file(form_path.string(), fields, records);
    expect(create_result.ok, "#1063: synthetic SCX table for object item fore color should be created");
}

void write_synthetic_form_table_for_object_highlight_back_color(const std::filesystem::path& form_path) {
    const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
        {.name = "OBJNAME", .type = 'C', .length = 24U},
        {.name = "NAME", .type = 'C', .length = 24U},
        {.name = "UNIQUEID", .type = 'C', .length = 32U},
        {.name = "HIGHLIGHTB", .type = 'C', .length = 16U}
    };
    const std::vector<std::vector<std::string>> records{
        {"lstCustomers", "lstCustomers", "one-guid", "16777215"},
        {"lstOrders", "lstOrders", "two-guid", "12632256"},
        {"lblStatus", "lblStatus", "three-guid", "255"},
        {"lstOther", "lstOther", "other-guid", "65280"}
    };

    const auto create_result = copperfin::vfp::create_dbf_table_file(form_path.string(), fields, records);
    expect(create_result.ok, "#1064: synthetic SCX table for object highlight back color should be created");
}

void write_synthetic_form_table_for_object_highlight_fore_color(const std::filesystem::path& form_path) {
    const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
        {.name = "OBJNAME", .type = 'C', .length = 24U},
        {.name = "NAME", .type = 'C', .length = 24U},
        {.name = "UNIQUEID", .type = 'C', .length = 32U},
        {.name = "HIGHLIGHTF", .type = 'C', .length = 16U}
    };
    const std::vector<std::vector<std::string>> records{
        {"lstCustomers", "lstCustomers", "one-guid", "0"},
        {"lstOrders", "lstOrders", "two-guid", "16777215"},
        {"lblStatus", "lblStatus", "three-guid", "255"},
        {"lstOther", "lstOther", "other-guid", "65280"}
    };

    const auto create_result = copperfin::vfp::create_dbf_table_file(form_path.string(), fields, records);
    expect(create_result.ok, "#1065: synthetic SCX table for object highlight fore color should be created");
}

void write_synthetic_form_table_for_object_back_color(const std::filesystem::path& form_path) {
    const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
        {.name = "OBJNAME", .type = 'C', .length = 24U},
        {.name = "NAME", .type = 'C', .length = 24U},
        {.name = "UNIQUEID", .type = 'C', .length = 32U},
        {.name = "BACKCOLOR", .type = 'C', .length = 16U}
    };
    const std::vector<std::vector<std::string>> records{
        {"lstCustomers", "lstCustomers", "one-guid", "16777215"},
        {"lstOrders", "lstOrders", "two-guid", "12632256"},
        {"lblStatus", "lblStatus", "three-guid", "255"},
        {"lstOther", "lstOther", "other-guid", "65280"}
    };

    const auto create_result = copperfin::vfp::create_dbf_table_file(form_path.string(), fields, records);
    expect(create_result.ok, "#1066: synthetic SCX table for object back color should be created");
}

void write_synthetic_form_table_for_object_fore_color(const std::filesystem::path& form_path) {
    const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
        {.name = "OBJNAME", .type = 'C', .length = 24U},
        {.name = "NAME", .type = 'C', .length = 24U},
        {.name = "UNIQUEID", .type = 'C', .length = 32U},
        {.name = "FORECOLOR", .type = 'C', .length = 16U}
    };
    const std::vector<std::vector<std::string>> records{
        {"lstCustomers", "lstCustomers", "one-guid", "0"},
        {"lstOrders", "lstOrders", "two-guid", "255"},
        {"lblStatus", "lblStatus", "three-guid", "16777215"},
        {"lstOther", "lstOther", "other-guid", "65280"}
    };

    const auto create_result = copperfin::vfp::create_dbf_table_file(form_path.string(), fields, records);
    expect(create_result.ok, "#1067: synthetic SCX table for object fore color should be created");
}

void write_synthetic_form_table_for_object_disabled_back_color(const std::filesystem::path& form_path) {
    const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
        {.name = "OBJNAME", .type = 'C', .length = 24U},
        {.name = "NAME", .type = 'C', .length = 24U},
        {.name = "UNIQUEID", .type = 'C', .length = 32U},
        {.name = "DISABLEDBA", .type = 'C', .length = 16U}
    };
    const std::vector<std::vector<std::string>> records{
        {"lstCustomers", "lstCustomers", "one-guid", "16777215"},
        {"lstOrders", "lstOrders", "two-guid", "12632256"},
        {"lblStatus", "lblStatus", "three-guid", "255"},
        {"lstOther", "lstOther", "other-guid", "65280"}
    };

    const auto create_result = copperfin::vfp::create_dbf_table_file(form_path.string(), fields, records);
    expect(create_result.ok, "#1068: synthetic SCX table for object disabled back color should be created");
}

void write_synthetic_form_table_for_object_disabled_fore_color(const std::filesystem::path& form_path) {
    const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
        {.name = "OBJNAME", .type = 'C', .length = 24U},
        {.name = "NAME", .type = 'C', .length = 24U},
        {.name = "UNIQUEID", .type = 'C', .length = 32U},
        {.name = "DISABLEDFO", .type = 'C', .length = 16U}
    };
    const std::vector<std::vector<std::string>> records{
        {"lstCustomers", "lstCustomers", "one-guid", "0"},
        {"lstOrders", "lstOrders", "two-guid", "255"},
        {"lblStatus", "lblStatus", "three-guid", "16777215"},
        {"lstOther", "lstOther", "other-guid", "65280"}
    };

    const auto create_result = copperfin::vfp::create_dbf_table_file(form_path.string(), fields, records);
    expect(create_result.ok, "#1069: synthetic SCX table for object disabled fore color should be created");
}

void write_synthetic_form_table_for_object_dynamic_back_color(const std::filesystem::path& form_path) {
    const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
        {.name = "OBJNAME", .type = 'C', .length = 24U},
        {.name = "NAME", .type = 'C', .length = 24U},
        {.name = "UNIQUEID", .type = 'C', .length = 32U},
        {.name = "DYNAMICBAC", .type = 'C', .length = 64U}
    };
    const std::vector<std::vector<std::string>> records{
        {"lstCustomers", "lstCustomers", "one-guid", "RGB(0,0,0)"},
        {"lstOrders", "lstOrders", "two-guid", "RGB(1,1,1)"},
        {"lblStatus", "lblStatus", "three-guid", "RGB(2,2,2)"},
        {"lstOther", "lstOther", "other-guid", "RGB(3,3,3)"}
    };

    const auto create_result = copperfin::vfp::create_dbf_table_file(form_path.string(), fields, records);
    expect(create_result.ok, "#1070: synthetic SCX table for object dynamic back color should be created");
}

void write_synthetic_form_table_for_object_dynamic_fore_color(const std::filesystem::path& form_path) {
    const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
        {.name = "OBJNAME", .type = 'C', .length = 24U},
        {.name = "NAME", .type = 'C', .length = 24U},
        {.name = "UNIQUEID", .type = 'C', .length = 32U},
        {.name = "DYNAMICFOR", .type = 'C', .length = 64U}
    };
    const std::vector<std::vector<std::string>> records{
        {"lstCustomers", "lstCustomers", "one-guid", "RGB(0,0,0)"},
        {"lstOrders", "lstOrders", "two-guid", "RGB(1,1,1)"},
        {"lblStatus", "lblStatus", "three-guid", "RGB(2,2,2)"},
        {"lstOther", "lstOther", "other-guid", "RGB(3,3,3)"}
    };

    const auto create_result = copperfin::vfp::create_dbf_table_file(form_path.string(), fields, records);
    expect(create_result.ok, "#1071: synthetic SCX table for object dynamic fore color should be created");
}

void test_studio_host_json_assigns_selected_back_color_by_stable_selectors(const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root = fs::temp_directory_path() / "copperfin_studio_host_selected_back_color_object_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path selected_back_color_path = temp_root / "selected_back_color.scx";
    write_synthetic_form_table_for_object_selected_back_color(selected_back_color_path);
    const auto selected_back_color_process = run_process_capture(
        studio_host_path,
        {
            "--path", selected_back_color_path.string(),
            "--selected-back-color-object",
            "--selected-back-color", "8421504",
            "--selected-back-color-target-object-name", "lstCustomers",
            "--selected-back-color-target-unique-id", "two-guid",
            "--json"
        },
        temp_root);
    expect(selected_back_color_process.exit_code == 0,
        "#1056: host object selected-back-color assignment should exit successfully");
    expect(visual_object_property(selected_back_color_path, "one-guid", "SELECTEDBACKCOLOR") == "8421504" &&
            visual_object_property(selected_back_color_path, "two-guid", "SELECTEDBACKCOLOR") == "8421504" &&
            visual_object_property(selected_back_color_path, "three-guid", "SELECTEDBACKCOLOR") == "255" &&
            visual_object_property(selected_back_color_path, "other-guid", "SELECTEDBACKCOLOR") == "65280",
        "#1056: host object selected-back-color assignment should assign selected numeric values and preserve unrelated objects");

    const fs::path missing_target_path = temp_root / "missing_target.scx";
    write_synthetic_form_table_for_object_selected_back_color(missing_target_path);
    const auto missing_target_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_target_path.string(),
            "--selected-back-color-object",
            "--selected-back-color", "8421504",
            "--selected-back-color-target-unique-id", "one-guid",
            "--selected-back-color-target-unique-id", "missing-guid",
            "--json"
        },
        temp_root);
    expect(missing_target_process.exit_code == 4,
        "#1056: missing-target host object selected-back-color assignment should return command failure");
    expect(visual_object_property(missing_target_path, "one-guid", "SELECTEDBACKCOLOR") == "16777215" &&
            visual_object_property(missing_target_path, "two-guid", "SELECTEDBACKCOLOR") == "12632256",
        "#1056: missing-target host object selected-back-color assignment should not mutate the asset");

    const fs::path missing_selector_path = temp_root / "missing_selector.scx";
    write_synthetic_form_table_for_object_selected_back_color(missing_selector_path);
    const auto missing_selector_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_selector_path.string(),
            "--selected-back-color-object",
            "--selected-back-color", "8421504",
            "--json"
        },
        temp_root);
    expect(missing_selector_process.exit_code == 2,
        "#1056: selected-back-color-object without target selectors should fail during launch parsing");
    expect(visual_object_property(missing_selector_path, "one-guid", "SELECTEDBACKCOLOR") == "16777215",
        "#1056: selected-back-color-object without target selectors should not mutate the asset");

    const fs::path missing_value_path = temp_root / "missing_value.scx";
    write_synthetic_form_table_for_object_selected_back_color(missing_value_path);
    const auto missing_value_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_value_path.string(),
            "--selected-back-color-object",
            "--selected-back-color-target-unique-id", "one-guid",
            "--json"
        },
        temp_root);
    expect(missing_value_process.exit_code == 2,
        "#1056: selected-back-color-object without selected-back-color should fail during launch parsing");
    expect(visual_object_property(missing_value_path, "one-guid", "SELECTEDBACKCOLOR") == "16777215",
        "#1056: selected-back-color-object without selected-back-color should not mutate the asset");

    const fs::path negative_path = temp_root / "negative.scx";
    write_synthetic_form_table_for_object_selected_back_color(negative_path);
    const auto negative_process = run_process_capture(
        studio_host_path,
        {
            "--path", negative_path.string(),
            "--selected-back-color-object",
            "--selected-back-color", "-1",
            "--selected-back-color-target-unique-id", "one-guid",
            "--json"
        },
        temp_root);
    expect(negative_process.exit_code == 2,
        "#1056: negative selected-back-color values should fail during launch parsing");
    expect(visual_object_property(negative_path, "one-guid", "SELECTEDBACKCOLOR") == "16777215",
        "#1056: negative selected-back-color values should not mutate the asset");

    const fs::path duplicate_path = temp_root / "duplicate.scx";
    write_synthetic_form_table_for_object_selected_back_color(duplicate_path);
    const auto duplicate_process = run_process_capture(
        studio_host_path,
        {
            "--path", duplicate_path.string(),
            "--selected-back-color-object",
            "--selected-back-color", "8421504",
            "--selected-back-color-target-unique-id", "one-guid",
            "--selected-back-color-target-object-name", "lstCustomers",
            "--json"
        },
        temp_root);
    expect(duplicate_process.exit_code == 4,
        "#1056: duplicate-target host object selected-back-color assignment should return command failure");
    expect(visual_object_property(duplicate_path, "one-guid", "SELECTEDBACKCOLOR") == "16777215",
        "#1056: duplicate-target host object selected-back-color assignment should not mutate the asset");

    const fs::path ambiguous_path = temp_root / "ambiguous.scx";
    write_synthetic_form_table_for_object_selected_back_color(ambiguous_path);
    const auto ambiguous_process = run_process_capture(
        studio_host_path,
        {
            "--path", ambiguous_path.string(),
            "--selected-back-color-object",
            "--display-value-object",
            "--selected-back-color", "8421504",
            "--selected-back-color-target-unique-id", "one-guid",
            "--display-value", "Bob",
            "--display-value-target-unique-id", "one-guid",
            "--json"
        },
        temp_root);
    expect(ambiguous_process.exit_code == 2,
        "#1056: selected-back-color-object plus display-value-object requests should fail during launch parsing");
    expect(visual_object_property(ambiguous_path, "one-guid", "SELECTEDBACKCOLOR") == "16777215",
        "#1056: selected-back-color-object ambiguity should not mutate the asset");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_assigns_selected_fore_color_by_stable_selectors(const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root = fs::temp_directory_path() / "copperfin_studio_host_selected_fore_color_object_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path selected_fore_color_path = temp_root / "selected_fore_color.scx";
    write_synthetic_form_table_for_object_selected_fore_color(selected_fore_color_path);
    const auto selected_fore_color_process = run_process_capture(
        studio_host_path,
        {
            "--path", selected_fore_color_path.string(),
            "--selected-fore-color-object",
            "--selected-fore-color", "8421504",
            "--selected-fore-color-target-object-name", "lstCustomers",
            "--selected-fore-color-target-unique-id", "two-guid",
            "--json"
        },
        temp_root);
    expect(selected_fore_color_process.exit_code == 0,
        "#1057: host object selected-fore-color assignment should exit successfully");
    expect(visual_object_property(selected_fore_color_path, "one-guid", "SELECTEDFORECOLOR") == "8421504" &&
            visual_object_property(selected_fore_color_path, "two-guid", "SELECTEDFORECOLOR") == "8421504" &&
            visual_object_property(selected_fore_color_path, "three-guid", "SELECTEDFORECOLOR") == "16777215" &&
            visual_object_property(selected_fore_color_path, "other-guid", "SELECTEDFORECOLOR") == "65280",
        "#1057: host object selected-fore-color assignment should assign selected numeric values and preserve unrelated objects");

    const fs::path missing_target_path = temp_root / "missing_target.scx";
    write_synthetic_form_table_for_object_selected_fore_color(missing_target_path);
    const auto missing_target_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_target_path.string(),
            "--selected-fore-color-object",
            "--selected-fore-color", "8421504",
            "--selected-fore-color-target-unique-id", "one-guid",
            "--selected-fore-color-target-unique-id", "missing-guid",
            "--json"
        },
        temp_root);
    expect(missing_target_process.exit_code == 4,
        "#1057: missing-target host object selected-fore-color assignment should return command failure");
    expect(visual_object_property(missing_target_path, "one-guid", "SELECTEDFORECOLOR") == "0" &&
            visual_object_property(missing_target_path, "two-guid", "SELECTEDFORECOLOR") == "255",
        "#1057: missing-target host object selected-fore-color assignment should not mutate the asset");

    const fs::path missing_selector_path = temp_root / "missing_selector.scx";
    write_synthetic_form_table_for_object_selected_fore_color(missing_selector_path);
    const auto missing_selector_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_selector_path.string(),
            "--selected-fore-color-object",
            "--selected-fore-color", "8421504",
            "--json"
        },
        temp_root);
    expect(missing_selector_process.exit_code == 2,
        "#1057: selected-fore-color-object without target selectors should fail during launch parsing");
    expect(visual_object_property(missing_selector_path, "one-guid", "SELECTEDFORECOLOR") == "0",
        "#1057: selected-fore-color-object without target selectors should not mutate the asset");

    const fs::path missing_value_path = temp_root / "missing_value.scx";
    write_synthetic_form_table_for_object_selected_fore_color(missing_value_path);
    const auto missing_value_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_value_path.string(),
            "--selected-fore-color-object",
            "--selected-fore-color-target-unique-id", "one-guid",
            "--json"
        },
        temp_root);
    expect(missing_value_process.exit_code == 2,
        "#1057: selected-fore-color-object without selected-fore-color should fail during launch parsing");
    expect(visual_object_property(missing_value_path, "one-guid", "SELECTEDFORECOLOR") == "0",
        "#1057: selected-fore-color-object without selected-fore-color should not mutate the asset");

    const fs::path negative_path = temp_root / "negative.scx";
    write_synthetic_form_table_for_object_selected_fore_color(negative_path);
    const auto negative_process = run_process_capture(
        studio_host_path,
        {
            "--path", negative_path.string(),
            "--selected-fore-color-object",
            "--selected-fore-color", "-1",
            "--selected-fore-color-target-unique-id", "one-guid",
            "--json"
        },
        temp_root);
    expect(negative_process.exit_code == 2,
        "#1057: negative selected-fore-color values should fail during launch parsing");
    expect(visual_object_property(negative_path, "one-guid", "SELECTEDFORECOLOR") == "0",
        "#1057: negative selected-fore-color values should not mutate the asset");

    const fs::path duplicate_path = temp_root / "duplicate.scx";
    write_synthetic_form_table_for_object_selected_fore_color(duplicate_path);
    const auto duplicate_process = run_process_capture(
        studio_host_path,
        {
            "--path", duplicate_path.string(),
            "--selected-fore-color-object",
            "--selected-fore-color", "8421504",
            "--selected-fore-color-target-unique-id", "one-guid",
            "--selected-fore-color-target-object-name", "lstCustomers",
            "--json"
        },
        temp_root);
    expect(duplicate_process.exit_code == 4,
        "#1057: duplicate-target host object selected-fore-color assignment should return command failure");
    expect(visual_object_property(duplicate_path, "one-guid", "SELECTEDFORECOLOR") == "0",
        "#1057: duplicate-target host object selected-fore-color assignment should not mutate the asset");

    const fs::path ambiguous_path = temp_root / "ambiguous.scx";
    write_synthetic_form_table_for_object_selected_fore_color(ambiguous_path);
    const auto ambiguous_process = run_process_capture(
        studio_host_path,
        {
            "--path", ambiguous_path.string(),
            "--selected-fore-color-object",
            "--selected-back-color-object",
            "--selected-fore-color", "8421504",
            "--selected-fore-color-target-unique-id", "one-guid",
            "--selected-back-color", "16777215",
            "--selected-back-color-target-unique-id", "one-guid",
            "--json"
        },
        temp_root);
    expect(ambiguous_process.exit_code == 2,
        "#1057: selected-fore-color-object plus selected-back-color-object requests should fail during launch parsing");
    expect(visual_object_property(ambiguous_path, "one-guid", "SELECTEDFORECOLOR") == "0",
        "#1057: selected-fore-color-object ambiguity should not mutate the asset");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_assigns_selected_item_back_color_by_stable_selectors(const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_selected_item_back_color_object_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path selected_item_back_color_path = temp_root / "selected_item_back_color.scx";
    write_synthetic_form_table_for_object_selected_item_back_color(selected_item_back_color_path);
    const auto selected_item_back_color_process = run_process_capture(
        studio_host_path,
        {
            "--path", selected_item_back_color_path.string(),
            "--selected-item-back-color-object",
            "--selected-item-back-color", "8421504",
            "--selected-item-back-color-target-object-name", "lstCustomers",
            "--selected-item-back-color-target-unique-id", "two-guid",
            "--json"
        },
        temp_root);
    expect(selected_item_back_color_process.exit_code == 0,
        "#1058: host object selected-item-back-color assignment should exit successfully");
    expect(visual_object_property(selected_item_back_color_path, "one-guid", "SELECTEDITEMBACKCOLOR") == "8421504" &&
            visual_object_property(selected_item_back_color_path, "two-guid", "SELECTEDITEMBACKCOLOR") == "8421504" &&
            visual_object_property(selected_item_back_color_path, "three-guid", "SELECTEDITEMBACKCOLOR") == "255" &&
            visual_object_property(selected_item_back_color_path, "other-guid", "SELECTEDITEMBACKCOLOR") == "65280",
        "#1058: host object selected-item-back-color assignment should assign selected numeric values and preserve unrelated objects");

    const fs::path missing_target_path = temp_root / "missing_target.scx";
    write_synthetic_form_table_for_object_selected_item_back_color(missing_target_path);
    const auto missing_target_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_target_path.string(),
            "--selected-item-back-color-object",
            "--selected-item-back-color", "8421504",
            "--selected-item-back-color-target-unique-id", "one-guid",
            "--selected-item-back-color-target-unique-id", "missing-guid",
            "--json"
        },
        temp_root);
    expect(missing_target_process.exit_code == 4,
        "#1058: missing-target host object selected-item-back-color assignment should return command failure");
    expect(visual_object_property(missing_target_path, "one-guid", "SELECTEDITEMBACKCOLOR") == "16777215" &&
            visual_object_property(missing_target_path, "two-guid", "SELECTEDITEMBACKCOLOR") == "12632256",
        "#1058: missing-target host object selected-item-back-color assignment should not mutate the asset");

    const fs::path missing_selector_path = temp_root / "missing_selector.scx";
    write_synthetic_form_table_for_object_selected_item_back_color(missing_selector_path);
    const auto missing_selector_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_selector_path.string(),
            "--selected-item-back-color-object",
            "--selected-item-back-color", "8421504",
            "--json"
        },
        temp_root);
    expect(missing_selector_process.exit_code == 2,
        "#1058: selected-item-back-color-object without target selectors should fail during launch parsing");
    expect(visual_object_property(missing_selector_path, "one-guid", "SELECTEDITEMBACKCOLOR") == "16777215",
        "#1058: selected-item-back-color-object without target selectors should not mutate the asset");

    const fs::path missing_value_path = temp_root / "missing_value.scx";
    write_synthetic_form_table_for_object_selected_item_back_color(missing_value_path);
    const auto missing_value_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_value_path.string(),
            "--selected-item-back-color-object",
            "--selected-item-back-color-target-unique-id", "one-guid",
            "--json"
        },
        temp_root);
    expect(missing_value_process.exit_code == 2,
        "#1058: selected-item-back-color-object without selected-item-back-color should fail during launch parsing");
    expect(visual_object_property(missing_value_path, "one-guid", "SELECTEDITEMBACKCOLOR") == "16777215",
        "#1058: selected-item-back-color-object without selected-item-back-color should not mutate the asset");

    const fs::path negative_path = temp_root / "negative.scx";
    write_synthetic_form_table_for_object_selected_item_back_color(negative_path);
    const auto negative_process = run_process_capture(
        studio_host_path,
        {
            "--path", negative_path.string(),
            "--selected-item-back-color-object",
            "--selected-item-back-color", "-1",
            "--selected-item-back-color-target-unique-id", "one-guid",
            "--json"
        },
        temp_root);
    expect(negative_process.exit_code == 2,
        "#1058: negative selected-item-back-color values should fail during launch parsing");
    expect(visual_object_property(negative_path, "one-guid", "SELECTEDITEMBACKCOLOR") == "16777215",
        "#1058: negative selected-item-back-color values should not mutate the asset");

    const fs::path duplicate_path = temp_root / "duplicate.scx";
    write_synthetic_form_table_for_object_selected_item_back_color(duplicate_path);
    const auto duplicate_process = run_process_capture(
        studio_host_path,
        {
            "--path", duplicate_path.string(),
            "--selected-item-back-color-object",
            "--selected-item-back-color", "8421504",
            "--selected-item-back-color-target-unique-id", "one-guid",
            "--selected-item-back-color-target-object-name", "lstCustomers",
            "--json"
        },
        temp_root);
    expect(duplicate_process.exit_code == 4,
        "#1058: duplicate-target host object selected-item-back-color assignment should return command failure");
    expect(visual_object_property(duplicate_path, "one-guid", "SELECTEDITEMBACKCOLOR") == "16777215",
        "#1058: duplicate-target host object selected-item-back-color assignment should not mutate the asset");

    const fs::path ambiguous_path = temp_root / "ambiguous.scx";
    write_synthetic_form_table_for_object_selected_item_back_color(ambiguous_path);
    const auto ambiguous_process = run_process_capture(
        studio_host_path,
        {
            "--path", ambiguous_path.string(),
            "--selected-item-back-color-object",
            "--selected-fore-color-object",
            "--selected-item-back-color", "8421504",
            "--selected-item-back-color-target-unique-id", "one-guid",
            "--selected-fore-color", "255",
            "--selected-fore-color-target-unique-id", "one-guid",
            "--json"
        },
        temp_root);
    expect(ambiguous_process.exit_code == 2,
        "#1058: selected-item-back-color-object plus selected-fore-color-object requests should fail during launch parsing");
    expect(visual_object_property(ambiguous_path, "one-guid", "SELECTEDITEMBACKCOLOR") == "16777215",
        "#1058: selected-item-back-color-object ambiguity should not mutate the asset");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_assigns_selected_item_fore_color_by_stable_selectors(const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_selected_item_fore_color_object_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path selected_item_fore_color_path = temp_root / "selected_item_fore_color.scx";
    write_synthetic_form_table_for_object_selected_item_fore_color(selected_item_fore_color_path);
    const auto selected_item_fore_color_process = run_process_capture(
        studio_host_path,
        {
            "--path", selected_item_fore_color_path.string(),
            "--selected-item-fore-color-object",
            "--selected-item-fore-color", "8421504",
            "--selected-item-fore-color-target-object-name", "lstCustomers",
            "--selected-item-fore-color-target-unique-id", "two-guid",
            "--json"
        },
        temp_root);
    expect(selected_item_fore_color_process.exit_code == 0,
        "#1059: host object selected-item-fore-color assignment should exit successfully");
    expect(visual_object_property(selected_item_fore_color_path, "one-guid", "SELECTEDITEMFORECOLOR") == "8421504" &&
            visual_object_property(selected_item_fore_color_path, "two-guid", "SELECTEDITEMFORECOLOR") == "8421504" &&
            visual_object_property(selected_item_fore_color_path, "three-guid", "SELECTEDITEMFORECOLOR") == "255" &&
            visual_object_property(selected_item_fore_color_path, "other-guid", "SELECTEDITEMFORECOLOR") == "65280",
        "#1059: host object selected-item-fore-color assignment should assign selected numeric values and preserve unrelated objects");

    const fs::path missing_target_path = temp_root / "missing_target.scx";
    write_synthetic_form_table_for_object_selected_item_fore_color(missing_target_path);
    const auto missing_target_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_target_path.string(),
            "--selected-item-fore-color-object",
            "--selected-item-fore-color", "8421504",
            "--selected-item-fore-color-target-unique-id", "one-guid",
            "--selected-item-fore-color-target-unique-id", "missing-guid",
            "--json"
        },
        temp_root);
    expect(missing_target_process.exit_code == 4,
        "#1059: missing-target host object selected-item-fore-color assignment should return command failure");
    expect(visual_object_property(missing_target_path, "one-guid", "SELECTEDITEMFORECOLOR") == "0" &&
            visual_object_property(missing_target_path, "two-guid", "SELECTEDITEMFORECOLOR") == "16777215",
        "#1059: missing-target host object selected-item-fore-color assignment should not mutate the asset");

    const fs::path missing_selector_path = temp_root / "missing_selector.scx";
    write_synthetic_form_table_for_object_selected_item_fore_color(missing_selector_path);
    const auto missing_selector_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_selector_path.string(),
            "--selected-item-fore-color-object",
            "--selected-item-fore-color", "8421504",
            "--json"
        },
        temp_root);
    expect(missing_selector_process.exit_code == 2,
        "#1059: selected-item-fore-color-object without target selectors should fail during launch parsing");
    expect(visual_object_property(missing_selector_path, "one-guid", "SELECTEDITEMFORECOLOR") == "0",
        "#1059: selected-item-fore-color-object without target selectors should not mutate the asset");

    const fs::path missing_value_path = temp_root / "missing_value.scx";
    write_synthetic_form_table_for_object_selected_item_fore_color(missing_value_path);
    const auto missing_value_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_value_path.string(),
            "--selected-item-fore-color-object",
            "--selected-item-fore-color-target-unique-id", "one-guid",
            "--json"
        },
        temp_root);
    expect(missing_value_process.exit_code == 2,
        "#1059: selected-item-fore-color-object without selected-item-fore-color should fail during launch parsing");
    expect(visual_object_property(missing_value_path, "one-guid", "SELECTEDITEMFORECOLOR") == "0",
        "#1059: selected-item-fore-color-object without selected-item-fore-color should not mutate the asset");

    const fs::path negative_path = temp_root / "negative.scx";
    write_synthetic_form_table_for_object_selected_item_fore_color(negative_path);
    const auto negative_process = run_process_capture(
        studio_host_path,
        {
            "--path", negative_path.string(),
            "--selected-item-fore-color-object",
            "--selected-item-fore-color", "-1",
            "--selected-item-fore-color-target-unique-id", "one-guid",
            "--json"
        },
        temp_root);
    expect(negative_process.exit_code == 2,
        "#1059: negative selected-item-fore-color values should fail during launch parsing");
    expect(visual_object_property(negative_path, "one-guid", "SELECTEDITEMFORECOLOR") == "0",
        "#1059: negative selected-item-fore-color values should not mutate the asset");

    const fs::path duplicate_path = temp_root / "duplicate.scx";
    write_synthetic_form_table_for_object_selected_item_fore_color(duplicate_path);
    const auto duplicate_process = run_process_capture(
        studio_host_path,
        {
            "--path", duplicate_path.string(),
            "--selected-item-fore-color-object",
            "--selected-item-fore-color", "8421504",
            "--selected-item-fore-color-target-unique-id", "one-guid",
            "--selected-item-fore-color-target-object-name", "lstCustomers",
            "--json"
        },
        temp_root);
    expect(duplicate_process.exit_code == 4,
        "#1059: duplicate-target host object selected-item-fore-color assignment should return command failure");
    expect(visual_object_property(duplicate_path, "one-guid", "SELECTEDITEMFORECOLOR") == "0",
        "#1059: duplicate-target host object selected-item-fore-color assignment should not mutate the asset");

    const fs::path ambiguous_path = temp_root / "ambiguous.scx";
    write_synthetic_form_table_for_object_selected_item_fore_color(ambiguous_path);
    const auto ambiguous_process = run_process_capture(
        studio_host_path,
        {
            "--path", ambiguous_path.string(),
            "--selected-item-fore-color-object",
            "--selected-item-back-color-object",
            "--selected-item-fore-color", "8421504",
            "--selected-item-fore-color-target-unique-id", "one-guid",
            "--selected-item-back-color", "65280",
            "--selected-item-back-color-target-unique-id", "one-guid",
            "--json"
        },
        temp_root);
    expect(ambiguous_process.exit_code == 2,
        "#1059: selected-item-fore-color-object plus selected-item-back-color-object requests should fail during launch parsing");
    expect(visual_object_property(ambiguous_path, "one-guid", "SELECTEDITEMFORECOLOR") == "0",
        "#1059: selected-item-fore-color-object ambiguity should not mutate the asset");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_assigns_disabled_item_back_color_by_stable_selectors(const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_disabled_item_back_color_object_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path disabled_item_back_color_path = temp_root / "disabled_item_back_color.scx";
    write_synthetic_form_table_for_object_disabled_item_back_color(disabled_item_back_color_path);
    const auto disabled_item_back_color_process = run_process_capture(
        studio_host_path,
        {
            "--path", disabled_item_back_color_path.string(),
            "--disabled-item-back-color-object",
            "--disabled-item-back-color", "8421504",
            "--disabled-item-back-color-target-object-name", "lstCustomers",
            "--disabled-item-back-color-target-unique-id", "two-guid",
            "--json"
        },
        temp_root);
    expect(disabled_item_back_color_process.exit_code == 0,
        "#1060: host object disabled-item-back-color assignment should exit successfully");
    expect(visual_object_property(disabled_item_back_color_path, "one-guid", "DISABLEDITEMBACKCOLOR") == "8421504" &&
            visual_object_property(disabled_item_back_color_path, "two-guid", "DISABLEDITEMBACKCOLOR") == "8421504" &&
            visual_object_property(disabled_item_back_color_path, "three-guid", "DISABLEDITEMBACKCOLOR") == "255" &&
            visual_object_property(disabled_item_back_color_path, "other-guid", "DISABLEDITEMBACKCOLOR") == "65280",
        "#1060: host object disabled-item-back-color assignment should assign selected numeric values and preserve unrelated objects");

    const fs::path missing_target_path = temp_root / "missing_target.scx";
    write_synthetic_form_table_for_object_disabled_item_back_color(missing_target_path);
    const auto missing_target_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_target_path.string(),
            "--disabled-item-back-color-object",
            "--disabled-item-back-color", "8421504",
            "--disabled-item-back-color-target-unique-id", "one-guid",
            "--disabled-item-back-color-target-unique-id", "missing-guid",
            "--json"
        },
        temp_root);
    expect(missing_target_process.exit_code == 4,
        "#1060: missing-target host object disabled-item-back-color assignment should return command failure");
    expect(visual_object_property(missing_target_path, "one-guid", "DISABLEDITEMBACKCOLOR") == "16777215" &&
            visual_object_property(missing_target_path, "two-guid", "DISABLEDITEMBACKCOLOR") == "12632256",
        "#1060: missing-target host object disabled-item-back-color assignment should not mutate the asset");

    const fs::path missing_selector_path = temp_root / "missing_selector.scx";
    write_synthetic_form_table_for_object_disabled_item_back_color(missing_selector_path);
    const auto missing_selector_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_selector_path.string(),
            "--disabled-item-back-color-object",
            "--disabled-item-back-color", "8421504",
            "--json"
        },
        temp_root);
    expect(missing_selector_process.exit_code == 2,
        "#1060: disabled-item-back-color-object without target selectors should fail during launch parsing");
    expect(visual_object_property(missing_selector_path, "one-guid", "DISABLEDITEMBACKCOLOR") == "16777215",
        "#1060: disabled-item-back-color-object without target selectors should not mutate the asset");

    const fs::path missing_value_path = temp_root / "missing_value.scx";
    write_synthetic_form_table_for_object_disabled_item_back_color(missing_value_path);
    const auto missing_value_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_value_path.string(),
            "--disabled-item-back-color-object",
            "--disabled-item-back-color-target-unique-id", "one-guid",
            "--json"
        },
        temp_root);
    expect(missing_value_process.exit_code == 2,
        "#1060: disabled-item-back-color-object without disabled-item-back-color should fail during launch parsing");
    expect(visual_object_property(missing_value_path, "one-guid", "DISABLEDITEMBACKCOLOR") == "16777215",
        "#1060: disabled-item-back-color-object without disabled-item-back-color should not mutate the asset");

    const fs::path negative_path = temp_root / "negative.scx";
    write_synthetic_form_table_for_object_disabled_item_back_color(negative_path);
    const auto negative_process = run_process_capture(
        studio_host_path,
        {
            "--path", negative_path.string(),
            "--disabled-item-back-color-object",
            "--disabled-item-back-color", "-1",
            "--disabled-item-back-color-target-unique-id", "one-guid",
            "--json"
        },
        temp_root);
    expect(negative_process.exit_code == 2,
        "#1060: negative disabled-item-back-color values should fail during launch parsing");
    expect(visual_object_property(negative_path, "one-guid", "DISABLEDITEMBACKCOLOR") == "16777215",
        "#1060: negative disabled-item-back-color values should not mutate the asset");

    const fs::path duplicate_path = temp_root / "duplicate.scx";
    write_synthetic_form_table_for_object_disabled_item_back_color(duplicate_path);
    const auto duplicate_process = run_process_capture(
        studio_host_path,
        {
            "--path", duplicate_path.string(),
            "--disabled-item-back-color-object",
            "--disabled-item-back-color", "8421504",
            "--disabled-item-back-color-target-unique-id", "one-guid",
            "--disabled-item-back-color-target-object-name", "lstCustomers",
            "--json"
        },
        temp_root);
    expect(duplicate_process.exit_code == 4,
        "#1060: duplicate-target host object disabled-item-back-color assignment should return command failure");
    expect(visual_object_property(duplicate_path, "one-guid", "DISABLEDITEMBACKCOLOR") == "16777215",
        "#1060: duplicate-target host object disabled-item-back-color assignment should not mutate the asset");

    const fs::path ambiguous_path = temp_root / "ambiguous.scx";
    write_synthetic_form_table_for_object_disabled_item_back_color(ambiguous_path);
    const auto ambiguous_process = run_process_capture(
        studio_host_path,
        {
            "--path", ambiguous_path.string(),
            "--disabled-item-back-color-object",
            "--selected-item-fore-color-object",
            "--disabled-item-back-color", "8421504",
            "--disabled-item-back-color-target-unique-id", "one-guid",
            "--selected-item-fore-color", "65280",
            "--selected-item-fore-color-target-unique-id", "one-guid",
            "--json"
        },
        temp_root);
    expect(ambiguous_process.exit_code == 2,
        "#1060: disabled-item-back-color-object plus selected-item-fore-color-object requests should fail during launch parsing");
    expect(visual_object_property(ambiguous_path, "one-guid", "DISABLEDITEMBACKCOLOR") == "16777215",
        "#1060: disabled-item-back-color-object ambiguity should not mutate the asset");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_assigns_disabled_item_fore_color_by_stable_selectors(const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_disabled_item_fore_color_object_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path disabled_item_fore_color_path = temp_root / "disabled_item_fore_color.scx";
    write_synthetic_form_table_for_object_disabled_item_fore_color(disabled_item_fore_color_path);
    const auto disabled_item_fore_color_process = run_process_capture(
        studio_host_path,
        {
            "--path", disabled_item_fore_color_path.string(),
            "--disabled-item-fore-color-object",
            "--disabled-item-fore-color", "8421504",
            "--disabled-item-fore-color-target-object-name", "lstCustomers",
            "--disabled-item-fore-color-target-unique-id", "two-guid",
            "--json"
        },
        temp_root);
    expect(disabled_item_fore_color_process.exit_code == 0,
        "#1061: host object disabled-item-fore-color assignment should exit successfully");
    expect(visual_object_property(disabled_item_fore_color_path, "one-guid", "DISABLEDITEMFORECOLOR") == "8421504" &&
            visual_object_property(disabled_item_fore_color_path, "two-guid", "DISABLEDITEMFORECOLOR") == "8421504" &&
            visual_object_property(disabled_item_fore_color_path, "three-guid", "DISABLEDITEMFORECOLOR") == "255" &&
            visual_object_property(disabled_item_fore_color_path, "other-guid", "DISABLEDITEMFORECOLOR") == "65280",
        "#1061: host object disabled-item-fore-color assignment should assign selected numeric values and preserve unrelated objects");

    const fs::path missing_target_path = temp_root / "missing_target.scx";
    write_synthetic_form_table_for_object_disabled_item_fore_color(missing_target_path);
    const auto missing_target_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_target_path.string(),
            "--disabled-item-fore-color-object",
            "--disabled-item-fore-color", "8421504",
            "--disabled-item-fore-color-target-unique-id", "one-guid",
            "--disabled-item-fore-color-target-unique-id", "missing-guid",
            "--json"
        },
        temp_root);
    expect(missing_target_process.exit_code == 4,
        "#1061: missing-target host object disabled-item-fore-color assignment should return command failure");
    expect(visual_object_property(missing_target_path, "one-guid", "DISABLEDITEMFORECOLOR") == "0" &&
            visual_object_property(missing_target_path, "two-guid", "DISABLEDITEMFORECOLOR") == "16777215",
        "#1061: missing-target host object disabled-item-fore-color assignment should not mutate the asset");

    const fs::path missing_selector_path = temp_root / "missing_selector.scx";
    write_synthetic_form_table_for_object_disabled_item_fore_color(missing_selector_path);
    const auto missing_selector_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_selector_path.string(),
            "--disabled-item-fore-color-object",
            "--disabled-item-fore-color", "8421504",
            "--json"
        },
        temp_root);
    expect(missing_selector_process.exit_code == 2,
        "#1061: disabled-item-fore-color-object without target selectors should fail during launch parsing");
    expect(visual_object_property(missing_selector_path, "one-guid", "DISABLEDITEMFORECOLOR") == "0",
        "#1061: disabled-item-fore-color-object without target selectors should not mutate the asset");

    const fs::path missing_value_path = temp_root / "missing_value.scx";
    write_synthetic_form_table_for_object_disabled_item_fore_color(missing_value_path);
    const auto missing_value_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_value_path.string(),
            "--disabled-item-fore-color-object",
            "--disabled-item-fore-color-target-unique-id", "one-guid",
            "--json"
        },
        temp_root);
    expect(missing_value_process.exit_code == 2,
        "#1061: disabled-item-fore-color-object without disabled-item-fore-color should fail during launch parsing");
    expect(visual_object_property(missing_value_path, "one-guid", "DISABLEDITEMFORECOLOR") == "0",
        "#1061: disabled-item-fore-color-object without disabled-item-fore-color should not mutate the asset");

    const fs::path negative_path = temp_root / "negative.scx";
    write_synthetic_form_table_for_object_disabled_item_fore_color(negative_path);
    const auto negative_process = run_process_capture(
        studio_host_path,
        {
            "--path", negative_path.string(),
            "--disabled-item-fore-color-object",
            "--disabled-item-fore-color", "-1",
            "--disabled-item-fore-color-target-unique-id", "one-guid",
            "--json"
        },
        temp_root);
    expect(negative_process.exit_code == 2,
        "#1061: negative disabled-item-fore-color values should fail during launch parsing");
    expect(visual_object_property(negative_path, "one-guid", "DISABLEDITEMFORECOLOR") == "0",
        "#1061: negative disabled-item-fore-color values should not mutate the asset");

    const fs::path duplicate_path = temp_root / "duplicate.scx";
    write_synthetic_form_table_for_object_disabled_item_fore_color(duplicate_path);
    const auto duplicate_process = run_process_capture(
        studio_host_path,
        {
            "--path", duplicate_path.string(),
            "--disabled-item-fore-color-object",
            "--disabled-item-fore-color", "8421504",
            "--disabled-item-fore-color-target-unique-id", "one-guid",
            "--disabled-item-fore-color-target-object-name", "lstCustomers",
            "--json"
        },
        temp_root);
    expect(duplicate_process.exit_code == 4,
        "#1061: duplicate-target host object disabled-item-fore-color assignment should return command failure");
    expect(visual_object_property(duplicate_path, "one-guid", "DISABLEDITEMFORECOLOR") == "0",
        "#1061: duplicate-target host object disabled-item-fore-color assignment should not mutate the asset");

    const fs::path ambiguous_path = temp_root / "ambiguous.scx";
    write_synthetic_form_table_for_object_disabled_item_fore_color(ambiguous_path);
    const auto ambiguous_process = run_process_capture(
        studio_host_path,
        {
            "--path", ambiguous_path.string(),
            "--disabled-item-fore-color-object",
            "--disabled-item-back-color-object",
            "--disabled-item-fore-color", "8421504",
            "--disabled-item-fore-color-target-unique-id", "one-guid",
            "--disabled-item-back-color", "65280",
            "--disabled-item-back-color-target-unique-id", "one-guid",
            "--json"
        },
        temp_root);
    expect(ambiguous_process.exit_code == 2,
        "#1061: disabled-item-fore-color-object plus disabled-item-back-color-object requests should fail during launch parsing");
    expect(visual_object_property(ambiguous_path, "one-guid", "DISABLEDITEMFORECOLOR") == "0",
        "#1061: disabled-item-fore-color-object ambiguity should not mutate the asset");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_assigns_item_back_color_by_stable_selectors(const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_item_back_color_object_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path item_back_color_path = temp_root / "item_back_color.scx";
    write_synthetic_form_table_for_object_item_back_color(item_back_color_path);
    const auto item_back_color_process = run_process_capture(
        studio_host_path,
        {
            "--path", item_back_color_path.string(),
            "--item-back-color-object",
            "--item-back-color", "8421504",
            "--item-back-color-target-object-name", "lstCustomers",
            "--item-back-color-target-unique-id", "two-guid",
            "--json"
        },
        temp_root);
    expect(item_back_color_process.exit_code == 0,
        "#1062: host object item-back-color assignment should exit successfully");
    expect(visual_object_property(item_back_color_path, "one-guid", "ITEMBACKCOLOR") == "8421504" &&
            visual_object_property(item_back_color_path, "two-guid", "ITEMBACKCOLOR") == "8421504" &&
            visual_object_property(item_back_color_path, "three-guid", "ITEMBACKCOLOR") == "255" &&
            visual_object_property(item_back_color_path, "other-guid", "ITEMBACKCOLOR") == "65280",
        "#1062: host object item-back-color assignment should assign selected numeric values and preserve unrelated objects");

    const fs::path missing_target_path = temp_root / "missing_target.scx";
    write_synthetic_form_table_for_object_item_back_color(missing_target_path);
    const auto missing_target_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_target_path.string(),
            "--item-back-color-object",
            "--item-back-color", "8421504",
            "--item-back-color-target-unique-id", "one-guid",
            "--item-back-color-target-unique-id", "missing-guid",
            "--json"
        },
        temp_root);
    expect(missing_target_process.exit_code == 4,
        "#1062: missing-target host object item-back-color assignment should return command failure");
    expect(visual_object_property(missing_target_path, "one-guid", "ITEMBACKCOLOR") == "16777215" &&
            visual_object_property(missing_target_path, "two-guid", "ITEMBACKCOLOR") == "12632256",
        "#1062: missing-target host object item-back-color assignment should not mutate the asset");

    const fs::path missing_selector_path = temp_root / "missing_selector.scx";
    write_synthetic_form_table_for_object_item_back_color(missing_selector_path);
    const auto missing_selector_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_selector_path.string(),
            "--item-back-color-object",
            "--item-back-color", "8421504",
            "--json"
        },
        temp_root);
    expect(missing_selector_process.exit_code == 2,
        "#1062: item-back-color-object without target selectors should fail during launch parsing");
    expect(visual_object_property(missing_selector_path, "one-guid", "ITEMBACKCOLOR") == "16777215",
        "#1062: item-back-color-object without target selectors should not mutate the asset");

    const fs::path missing_value_path = temp_root / "missing_value.scx";
    write_synthetic_form_table_for_object_item_back_color(missing_value_path);
    const auto missing_value_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_value_path.string(),
            "--item-back-color-object",
            "--item-back-color-target-unique-id", "one-guid",
            "--json"
        },
        temp_root);
    expect(missing_value_process.exit_code == 2,
        "#1062: item-back-color-object without item-back-color should fail during launch parsing");
    expect(visual_object_property(missing_value_path, "one-guid", "ITEMBACKCOLOR") == "16777215",
        "#1062: item-back-color-object without item-back-color should not mutate the asset");

    const fs::path negative_path = temp_root / "negative.scx";
    write_synthetic_form_table_for_object_item_back_color(negative_path);
    const auto negative_process = run_process_capture(
        studio_host_path,
        {
            "--path", negative_path.string(),
            "--item-back-color-object",
            "--item-back-color", "-1",
            "--item-back-color-target-unique-id", "one-guid",
            "--json"
        },
        temp_root);
    expect(negative_process.exit_code == 2,
        "#1062: negative item-back-color values should fail during launch parsing");
    expect(visual_object_property(negative_path, "one-guid", "ITEMBACKCOLOR") == "16777215",
        "#1062: negative item-back-color values should not mutate the asset");

    const fs::path duplicate_path = temp_root / "duplicate.scx";
    write_synthetic_form_table_for_object_item_back_color(duplicate_path);
    const auto duplicate_process = run_process_capture(
        studio_host_path,
        {
            "--path", duplicate_path.string(),
            "--item-back-color-object",
            "--item-back-color", "8421504",
            "--item-back-color-target-unique-id", "one-guid",
            "--item-back-color-target-object-name", "lstCustomers",
            "--json"
        },
        temp_root);
    expect(duplicate_process.exit_code == 4,
        "#1062: duplicate-target host object item-back-color assignment should return command failure");
    expect(visual_object_property(duplicate_path, "one-guid", "ITEMBACKCOLOR") == "16777215",
        "#1062: duplicate-target host object item-back-color assignment should not mutate the asset");

    const fs::path ambiguous_path = temp_root / "ambiguous.scx";
    write_synthetic_form_table_for_object_item_back_color(ambiguous_path);
    const auto ambiguous_process = run_process_capture(
        studio_host_path,
        {
            "--path", ambiguous_path.string(),
            "--item-back-color-object",
            "--disabled-item-fore-color-object",
            "--item-back-color", "8421504",
            "--item-back-color-target-unique-id", "one-guid",
            "--disabled-item-fore-color", "65280",
            "--disabled-item-fore-color-target-unique-id", "one-guid",
            "--json"
        },
        temp_root);
    expect(ambiguous_process.exit_code == 2,
        "#1062: item-back-color-object plus disabled-item-fore-color-object requests should fail during launch parsing");
    expect(visual_object_property(ambiguous_path, "one-guid", "ITEMBACKCOLOR") == "16777215",
        "#1062: item-back-color-object ambiguity should not mutate the asset");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_assigns_item_fore_color_by_stable_selectors(const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_item_fore_color_object_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path item_fore_color_path = temp_root / "item_fore_color.scx";
    write_synthetic_form_table_for_object_item_fore_color(item_fore_color_path);
    const auto item_fore_color_process = run_process_capture(
        studio_host_path,
        {
            "--path", item_fore_color_path.string(),
            "--item-fore-color-object",
            "--item-fore-color", "8421504",
            "--item-fore-color-target-object-name", "lstCustomers",
            "--item-fore-color-target-unique-id", "two-guid",
            "--json"
        },
        temp_root);
    expect(item_fore_color_process.exit_code == 0,
        "#1063: host object item-fore-color assignment should exit successfully");
    expect(visual_object_property(item_fore_color_path, "one-guid", "ITEMFORECOLOR") == "8421504" &&
            visual_object_property(item_fore_color_path, "two-guid", "ITEMFORECOLOR") == "8421504" &&
            visual_object_property(item_fore_color_path, "three-guid", "ITEMFORECOLOR") == "255" &&
            visual_object_property(item_fore_color_path, "other-guid", "ITEMFORECOLOR") == "65280",
        "#1063: host object item-fore-color assignment should assign selected numeric values and preserve unrelated objects");

    const fs::path missing_target_path = temp_root / "missing_target.scx";
    write_synthetic_form_table_for_object_item_fore_color(missing_target_path);
    const auto missing_target_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_target_path.string(),
            "--item-fore-color-object",
            "--item-fore-color", "8421504",
            "--item-fore-color-target-unique-id", "one-guid",
            "--item-fore-color-target-unique-id", "missing-guid",
            "--json"
        },
        temp_root);
    expect(missing_target_process.exit_code == 4,
        "#1063: missing-target host object item-fore-color assignment should return command failure");
    expect(visual_object_property(missing_target_path, "one-guid", "ITEMFORECOLOR") == "0" &&
            visual_object_property(missing_target_path, "two-guid", "ITEMFORECOLOR") == "16777215",
        "#1063: missing-target host object item-fore-color assignment should not mutate the asset");

    const fs::path missing_selector_path = temp_root / "missing_selector.scx";
    write_synthetic_form_table_for_object_item_fore_color(missing_selector_path);
    const auto missing_selector_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_selector_path.string(),
            "--item-fore-color-object",
            "--item-fore-color", "8421504",
            "--json"
        },
        temp_root);
    expect(missing_selector_process.exit_code == 2,
        "#1063: item-fore-color-object without target selectors should fail during launch parsing");
    expect(visual_object_property(missing_selector_path, "one-guid", "ITEMFORECOLOR") == "0",
        "#1063: item-fore-color-object without target selectors should not mutate the asset");

    const fs::path missing_value_path = temp_root / "missing_value.scx";
    write_synthetic_form_table_for_object_item_fore_color(missing_value_path);
    const auto missing_value_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_value_path.string(),
            "--item-fore-color-object",
            "--item-fore-color-target-unique-id", "one-guid",
            "--json"
        },
        temp_root);
    expect(missing_value_process.exit_code == 2,
        "#1063: item-fore-color-object without item-fore-color should fail during launch parsing");
    expect(visual_object_property(missing_value_path, "one-guid", "ITEMFORECOLOR") == "0",
        "#1063: item-fore-color-object without item-fore-color should not mutate the asset");

    const fs::path negative_path = temp_root / "negative.scx";
    write_synthetic_form_table_for_object_item_fore_color(negative_path);
    const auto negative_process = run_process_capture(
        studio_host_path,
        {
            "--path", negative_path.string(),
            "--item-fore-color-object",
            "--item-fore-color", "-1",
            "--item-fore-color-target-unique-id", "one-guid",
            "--json"
        },
        temp_root);
    expect(negative_process.exit_code == 2,
        "#1063: negative item-fore-color values should fail during launch parsing");
    expect(visual_object_property(negative_path, "one-guid", "ITEMFORECOLOR") == "0",
        "#1063: negative item-fore-color values should not mutate the asset");

    const fs::path duplicate_path = temp_root / "duplicate.scx";
    write_synthetic_form_table_for_object_item_fore_color(duplicate_path);
    const auto duplicate_process = run_process_capture(
        studio_host_path,
        {
            "--path", duplicate_path.string(),
            "--item-fore-color-object",
            "--item-fore-color", "8421504",
            "--item-fore-color-target-unique-id", "one-guid",
            "--item-fore-color-target-object-name", "lstCustomers",
            "--json"
        },
        temp_root);
    expect(duplicate_process.exit_code == 4,
        "#1063: duplicate-target host object item-fore-color assignment should return command failure");
    expect(visual_object_property(duplicate_path, "one-guid", "ITEMFORECOLOR") == "0",
        "#1063: duplicate-target host object item-fore-color assignment should not mutate the asset");

    const fs::path ambiguous_path = temp_root / "ambiguous.scx";
    write_synthetic_form_table_for_object_item_fore_color(ambiguous_path);
    const auto ambiguous_process = run_process_capture(
        studio_host_path,
        {
            "--path", ambiguous_path.string(),
            "--item-fore-color-object",
            "--item-back-color-object",
            "--item-fore-color", "8421504",
            "--item-fore-color-target-unique-id", "one-guid",
            "--item-back-color", "65280",
            "--item-back-color-target-unique-id", "one-guid",
            "--json"
        },
        temp_root);
    expect(ambiguous_process.exit_code == 2,
        "#1063: item-fore-color-object plus item-back-color-object requests should fail during launch parsing");
    expect(visual_object_property(ambiguous_path, "one-guid", "ITEMFORECOLOR") == "0",
        "#1063: item-fore-color-object ambiguity should not mutate the asset");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_assigns_highlight_back_color_by_stable_selectors(const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_highlight_back_color_object_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path highlight_back_color_path = temp_root / "highlight_back_color.scx";
    write_synthetic_form_table_for_object_highlight_back_color(highlight_back_color_path);
    const auto highlight_back_color_process = run_process_capture(
        studio_host_path,
        {
            "--path", highlight_back_color_path.string(),
            "--highlight-back-color-object",
            "--highlight-back-color", "8421504",
            "--highlight-back-color-target-object-name", "lstCustomers",
            "--highlight-back-color-target-unique-id", "two-guid",
            "--json"
        },
        temp_root);
    expect(highlight_back_color_process.exit_code == 0,
        "#1064: host object highlight-back-color assignment should exit successfully");
    expect(visual_object_property(highlight_back_color_path, "one-guid", "HIGHLIGHTBACKCOLOR") == "8421504" &&
            visual_object_property(highlight_back_color_path, "two-guid", "HIGHLIGHTBACKCOLOR") == "8421504" &&
            visual_object_property(highlight_back_color_path, "three-guid", "HIGHLIGHTBACKCOLOR") == "255" &&
            visual_object_property(highlight_back_color_path, "other-guid", "HIGHLIGHTBACKCOLOR") == "65280",
        "#1064: host object highlight-back-color assignment should assign selected numeric values and preserve unrelated objects");

    const fs::path missing_target_path = temp_root / "missing_target.scx";
    write_synthetic_form_table_for_object_highlight_back_color(missing_target_path);
    const auto missing_target_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_target_path.string(),
            "--highlight-back-color-object",
            "--highlight-back-color", "8421504",
            "--highlight-back-color-target-unique-id", "one-guid",
            "--highlight-back-color-target-unique-id", "missing-guid",
            "--json"
        },
        temp_root);
    expect(missing_target_process.exit_code == 4,
        "#1064: missing-target host object highlight-back-color assignment should return command failure");
    expect(visual_object_property(missing_target_path, "one-guid", "HIGHLIGHTBACKCOLOR") == "16777215" &&
            visual_object_property(missing_target_path, "two-guid", "HIGHLIGHTBACKCOLOR") == "12632256",
        "#1064: missing-target host object highlight-back-color assignment should not mutate the asset");

    const fs::path missing_selector_path = temp_root / "missing_selector.scx";
    write_synthetic_form_table_for_object_highlight_back_color(missing_selector_path);
    const auto missing_selector_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_selector_path.string(),
            "--highlight-back-color-object",
            "--highlight-back-color", "8421504",
            "--json"
        },
        temp_root);
    expect(missing_selector_process.exit_code == 2,
        "#1064: highlight-back-color-object without target selectors should fail during launch parsing");
    expect(visual_object_property(missing_selector_path, "one-guid", "HIGHLIGHTBACKCOLOR") == "16777215",
        "#1064: highlight-back-color-object without target selectors should not mutate the asset");

    const fs::path missing_value_path = temp_root / "missing_value.scx";
    write_synthetic_form_table_for_object_highlight_back_color(missing_value_path);
    const auto missing_value_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_value_path.string(),
            "--highlight-back-color-object",
            "--highlight-back-color-target-unique-id", "one-guid",
            "--json"
        },
        temp_root);
    expect(missing_value_process.exit_code == 2,
        "#1064: highlight-back-color-object without highlight-back-color should fail during launch parsing");
    expect(visual_object_property(missing_value_path, "one-guid", "HIGHLIGHTBACKCOLOR") == "16777215",
        "#1064: highlight-back-color-object without highlight-back-color should not mutate the asset");

    const fs::path negative_path = temp_root / "negative.scx";
    write_synthetic_form_table_for_object_highlight_back_color(negative_path);
    const auto negative_process = run_process_capture(
        studio_host_path,
        {
            "--path", negative_path.string(),
            "--highlight-back-color-object",
            "--highlight-back-color", "-1",
            "--highlight-back-color-target-unique-id", "one-guid",
            "--json"
        },
        temp_root);
    expect(negative_process.exit_code == 2,
        "#1064: negative highlight-back-color values should fail during launch parsing");
    expect(visual_object_property(negative_path, "one-guid", "HIGHLIGHTBACKCOLOR") == "16777215",
        "#1064: negative highlight-back-color values should not mutate the asset");

    const fs::path duplicate_path = temp_root / "duplicate.scx";
    write_synthetic_form_table_for_object_highlight_back_color(duplicate_path);
    const auto duplicate_process = run_process_capture(
        studio_host_path,
        {
            "--path", duplicate_path.string(),
            "--highlight-back-color-object",
            "--highlight-back-color", "8421504",
            "--highlight-back-color-target-unique-id", "one-guid",
            "--highlight-back-color-target-object-name", "lstCustomers",
            "--json"
        },
        temp_root);
    expect(duplicate_process.exit_code == 4,
        "#1064: duplicate-target host object highlight-back-color assignment should return command failure");
    expect(visual_object_property(duplicate_path, "one-guid", "HIGHLIGHTBACKCOLOR") == "16777215",
        "#1064: duplicate-target host object highlight-back-color assignment should not mutate the asset");

    const fs::path ambiguous_path = temp_root / "ambiguous.scx";
    write_synthetic_form_table_for_object_highlight_back_color(ambiguous_path);
    const auto ambiguous_process = run_process_capture(
        studio_host_path,
        {
            "--path", ambiguous_path.string(),
            "--highlight-back-color-object",
            "--item-fore-color-object",
            "--highlight-back-color", "8421504",
            "--highlight-back-color-target-unique-id", "one-guid",
            "--item-fore-color", "65280",
            "--item-fore-color-target-unique-id", "one-guid",
            "--json"
        },
        temp_root);
    expect(ambiguous_process.exit_code == 2,
        "#1064: highlight-back-color-object plus item-fore-color-object requests should fail during launch parsing");
    expect(visual_object_property(ambiguous_path, "one-guid", "HIGHLIGHTBACKCOLOR") == "16777215",
        "#1064: highlight-back-color-object ambiguity should not mutate the asset");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_assigns_highlight_fore_color_by_stable_selectors(const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_highlight_fore_color_object_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path highlight_fore_color_path = temp_root / "highlight_fore_color.scx";
    write_synthetic_form_table_for_object_highlight_fore_color(highlight_fore_color_path);
    const auto highlight_fore_color_process = run_process_capture(
        studio_host_path,
        {
            "--path", highlight_fore_color_path.string(),
            "--highlight-fore-color-object",
            "--highlight-fore-color", "8421504",
            "--highlight-fore-color-target-object-name", "lstCustomers",
            "--highlight-fore-color-target-unique-id", "two-guid",
            "--json"
        },
        temp_root);
    expect(highlight_fore_color_process.exit_code == 0,
        "#1065: host object highlight-fore-color assignment should exit successfully");
    expect(visual_object_property(highlight_fore_color_path, "one-guid", "HIGHLIGHTFORECOLOR") == "8421504" &&
            visual_object_property(highlight_fore_color_path, "two-guid", "HIGHLIGHTFORECOLOR") == "8421504" &&
            visual_object_property(highlight_fore_color_path, "three-guid", "HIGHLIGHTFORECOLOR") == "255" &&
            visual_object_property(highlight_fore_color_path, "other-guid", "HIGHLIGHTFORECOLOR") == "65280",
        "#1065: host object highlight-fore-color assignment should assign selected numeric values and preserve unrelated objects");

    const fs::path missing_target_path = temp_root / "missing_target.scx";
    write_synthetic_form_table_for_object_highlight_fore_color(missing_target_path);
    const auto missing_target_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_target_path.string(),
            "--highlight-fore-color-object",
            "--highlight-fore-color", "8421504",
            "--highlight-fore-color-target-unique-id", "one-guid",
            "--highlight-fore-color-target-unique-id", "missing-guid",
            "--json"
        },
        temp_root);
    expect(missing_target_process.exit_code == 4,
        "#1065: missing-target host object highlight-fore-color assignment should return command failure");
    expect(visual_object_property(missing_target_path, "one-guid", "HIGHLIGHTFORECOLOR") == "0" &&
            visual_object_property(missing_target_path, "two-guid", "HIGHLIGHTFORECOLOR") == "16777215",
        "#1065: missing-target host object highlight-fore-color assignment should not mutate the asset");

    const fs::path missing_selector_path = temp_root / "missing_selector.scx";
    write_synthetic_form_table_for_object_highlight_fore_color(missing_selector_path);
    const auto missing_selector_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_selector_path.string(),
            "--highlight-fore-color-object",
            "--highlight-fore-color", "8421504",
            "--json"
        },
        temp_root);
    expect(missing_selector_process.exit_code == 2,
        "#1065: highlight-fore-color-object without target selectors should fail during launch parsing");
    expect(visual_object_property(missing_selector_path, "one-guid", "HIGHLIGHTFORECOLOR") == "0",
        "#1065: highlight-fore-color-object without target selectors should not mutate the asset");

    const fs::path missing_value_path = temp_root / "missing_value.scx";
    write_synthetic_form_table_for_object_highlight_fore_color(missing_value_path);
    const auto missing_value_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_value_path.string(),
            "--highlight-fore-color-object",
            "--highlight-fore-color-target-unique-id", "one-guid",
            "--json"
        },
        temp_root);
    expect(missing_value_process.exit_code == 2,
        "#1065: highlight-fore-color-object without highlight-fore-color should fail during launch parsing");
    expect(visual_object_property(missing_value_path, "one-guid", "HIGHLIGHTFORECOLOR") == "0",
        "#1065: highlight-fore-color-object without highlight-fore-color should not mutate the asset");

    const fs::path negative_path = temp_root / "negative.scx";
    write_synthetic_form_table_for_object_highlight_fore_color(negative_path);
    const auto negative_process = run_process_capture(
        studio_host_path,
        {
            "--path", negative_path.string(),
            "--highlight-fore-color-object",
            "--highlight-fore-color", "-1",
            "--highlight-fore-color-target-unique-id", "one-guid",
            "--json"
        },
        temp_root);
    expect(negative_process.exit_code == 2,
        "#1065: negative highlight-fore-color values should fail during launch parsing");
    expect(visual_object_property(negative_path, "one-guid", "HIGHLIGHTFORECOLOR") == "0",
        "#1065: negative highlight-fore-color values should not mutate the asset");

    const fs::path duplicate_path = temp_root / "duplicate.scx";
    write_synthetic_form_table_for_object_highlight_fore_color(duplicate_path);
    const auto duplicate_process = run_process_capture(
        studio_host_path,
        {
            "--path", duplicate_path.string(),
            "--highlight-fore-color-object",
            "--highlight-fore-color", "8421504",
            "--highlight-fore-color-target-unique-id", "one-guid",
            "--highlight-fore-color-target-object-name", "lstCustomers",
            "--json"
        },
        temp_root);
    expect(duplicate_process.exit_code == 4,
        "#1065: duplicate-target host object highlight-fore-color assignment should return command failure");
    expect(visual_object_property(duplicate_path, "one-guid", "HIGHLIGHTFORECOLOR") == "0",
        "#1065: duplicate-target host object highlight-fore-color assignment should not mutate the asset");

    const fs::path ambiguous_path = temp_root / "ambiguous.scx";
    write_synthetic_form_table_for_object_highlight_fore_color(ambiguous_path);
    const auto ambiguous_process = run_process_capture(
        studio_host_path,
        {
            "--path", ambiguous_path.string(),
            "--highlight-fore-color-object",
            "--highlight-back-color-object",
            "--highlight-fore-color", "8421504",
            "--highlight-fore-color-target-unique-id", "one-guid",
            "--highlight-back-color", "65280",
            "--highlight-back-color-target-unique-id", "one-guid",
            "--json"
        },
        temp_root);
    expect(ambiguous_process.exit_code == 2,
        "#1065: highlight-fore-color-object plus highlight-back-color-object requests should fail during launch parsing");
    expect(visual_object_property(ambiguous_path, "one-guid", "HIGHLIGHTFORECOLOR") == "0",
        "#1065: highlight-fore-color-object ambiguity should not mutate the asset");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_assigns_back_color_by_stable_selectors(const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_back_color_object_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path back_color_path = temp_root / "back_color.scx";
    write_synthetic_form_table_for_object_back_color(back_color_path);
    const auto back_color_process = run_process_capture(
        studio_host_path,
        {
            "--path", back_color_path.string(),
            "--back-color-object",
            "--back-color", "8421504",
            "--back-color-target-object-name", "lstCustomers",
            "--back-color-target-unique-id", "two-guid",
            "--json"
        },
        temp_root);
    expect(back_color_process.exit_code == 0,
        "#1066: host object back-color assignment should exit successfully");
    expect(visual_object_property(back_color_path, "one-guid", "BACKCOLOR") == "8421504" &&
            visual_object_property(back_color_path, "two-guid", "BACKCOLOR") == "8421504" &&
            visual_object_property(back_color_path, "three-guid", "BACKCOLOR") == "255" &&
            visual_object_property(back_color_path, "other-guid", "BACKCOLOR") == "65280",
        "#1066: host object back-color assignment should assign selected numeric values and preserve unrelated objects");

    const fs::path missing_target_path = temp_root / "missing_target.scx";
    write_synthetic_form_table_for_object_back_color(missing_target_path);
    const auto missing_target_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_target_path.string(),
            "--back-color-object",
            "--back-color", "8421504",
            "--back-color-target-unique-id", "one-guid",
            "--back-color-target-unique-id", "missing-guid",
            "--json"
        },
        temp_root);
    expect(missing_target_process.exit_code == 4,
        "#1066: missing-target host object back-color assignment should return command failure");
    expect(visual_object_property(missing_target_path, "one-guid", "BACKCOLOR") == "16777215" &&
            visual_object_property(missing_target_path, "two-guid", "BACKCOLOR") == "12632256",
        "#1066: missing-target host object back-color assignment should not mutate the asset");

    const fs::path missing_selector_path = temp_root / "missing_selector.scx";
    write_synthetic_form_table_for_object_back_color(missing_selector_path);
    const auto missing_selector_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_selector_path.string(),
            "--back-color-object",
            "--back-color", "8421504",
            "--json"
        },
        temp_root);
    expect(missing_selector_process.exit_code == 2,
        "#1066: back-color-object without target selectors should fail during launch parsing");
    expect(visual_object_property(missing_selector_path, "one-guid", "BACKCOLOR") == "16777215",
        "#1066: back-color-object without target selectors should not mutate the asset");

    const fs::path missing_value_path = temp_root / "missing_value.scx";
    write_synthetic_form_table_for_object_back_color(missing_value_path);
    const auto missing_value_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_value_path.string(),
            "--back-color-object",
            "--back-color-target-unique-id", "one-guid",
            "--json"
        },
        temp_root);
    expect(missing_value_process.exit_code == 2,
        "#1066: back-color-object without back-color should fail during launch parsing");
    expect(visual_object_property(missing_value_path, "one-guid", "BACKCOLOR") == "16777215",
        "#1066: back-color-object without back-color should not mutate the asset");

    const fs::path negative_path = temp_root / "negative.scx";
    write_synthetic_form_table_for_object_back_color(negative_path);
    const auto negative_process = run_process_capture(
        studio_host_path,
        {
            "--path", negative_path.string(),
            "--back-color-object",
            "--back-color", "-1",
            "--back-color-target-unique-id", "one-guid",
            "--json"
        },
        temp_root);
    expect(negative_process.exit_code == 2,
        "#1066: negative back-color values should fail during launch parsing");
    expect(visual_object_property(negative_path, "one-guid", "BACKCOLOR") == "16777215",
        "#1066: negative back-color values should not mutate the asset");

    const fs::path duplicate_path = temp_root / "duplicate.scx";
    write_synthetic_form_table_for_object_back_color(duplicate_path);
    const auto duplicate_process = run_process_capture(
        studio_host_path,
        {
            "--path", duplicate_path.string(),
            "--back-color-object",
            "--back-color", "8421504",
            "--back-color-target-unique-id", "one-guid",
            "--back-color-target-object-name", "lstCustomers",
            "--json"
        },
        temp_root);
    expect(duplicate_process.exit_code == 4,
        "#1066: duplicate-target host object back-color assignment should return command failure");
    expect(visual_object_property(duplicate_path, "one-guid", "BACKCOLOR") == "16777215",
        "#1066: duplicate-target host object back-color assignment should not mutate the asset");

    const fs::path ambiguous_path = temp_root / "ambiguous.scx";
    write_synthetic_form_table_for_object_back_color(ambiguous_path);
    const auto ambiguous_process = run_process_capture(
        studio_host_path,
        {
            "--path", ambiguous_path.string(),
            "--back-color-object",
            "--highlight-fore-color-object",
            "--back-color", "8421504",
            "--back-color-target-unique-id", "one-guid",
            "--highlight-fore-color", "65280",
            "--highlight-fore-color-target-unique-id", "one-guid",
            "--json"
        },
        temp_root);
    expect(ambiguous_process.exit_code == 2,
        "#1066: back-color-object plus highlight-fore-color-object requests should fail during launch parsing");
    expect(visual_object_property(ambiguous_path, "one-guid", "BACKCOLOR") == "16777215",
        "#1066: back-color-object ambiguity should not mutate the asset");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_assigns_fore_color_by_stable_selectors(const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_fore_color_object_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path fore_color_path = temp_root / "fore_color.scx";
    write_synthetic_form_table_for_object_fore_color(fore_color_path);
    const auto fore_color_process = run_process_capture(
        studio_host_path,
        {
            "--path", fore_color_path.string(),
            "--fore-color-object",
            "--fore-color", "8421504",
            "--fore-color-target-object-name", "lstCustomers",
            "--fore-color-target-unique-id", "two-guid",
            "--json"
        },
        temp_root);
    expect(fore_color_process.exit_code == 0,
        "#1067: host object fore-color assignment should exit successfully");
    expect(visual_object_property(fore_color_path, "one-guid", "FORECOLOR") == "8421504" &&
            visual_object_property(fore_color_path, "two-guid", "FORECOLOR") == "8421504" &&
            visual_object_property(fore_color_path, "three-guid", "FORECOLOR") == "16777215" &&
            visual_object_property(fore_color_path, "other-guid", "FORECOLOR") == "65280",
        "#1067: host object fore-color assignment should assign selected numeric values and preserve unrelated objects");

    const fs::path missing_target_path = temp_root / "missing_target.scx";
    write_synthetic_form_table_for_object_fore_color(missing_target_path);
    const auto missing_target_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_target_path.string(),
            "--fore-color-object",
            "--fore-color", "8421504",
            "--fore-color-target-unique-id", "one-guid",
            "--fore-color-target-unique-id", "missing-guid",
            "--json"
        },
        temp_root);
    expect(missing_target_process.exit_code == 4,
        "#1067: missing-target host object fore-color assignment should return command failure");
    expect(visual_object_property(missing_target_path, "one-guid", "FORECOLOR") == "0" &&
            visual_object_property(missing_target_path, "two-guid", "FORECOLOR") == "255",
        "#1067: missing-target host object fore-color assignment should not mutate the asset");

    const fs::path missing_selector_path = temp_root / "missing_selector.scx";
    write_synthetic_form_table_for_object_fore_color(missing_selector_path);
    const auto missing_selector_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_selector_path.string(),
            "--fore-color-object",
            "--fore-color", "8421504",
            "--json"
        },
        temp_root);
    expect(missing_selector_process.exit_code == 2,
        "#1067: fore-color-object without target selectors should fail during launch parsing");
    expect(visual_object_property(missing_selector_path, "one-guid", "FORECOLOR") == "0",
        "#1067: fore-color-object without target selectors should not mutate the asset");

    const fs::path missing_value_path = temp_root / "missing_value.scx";
    write_synthetic_form_table_for_object_fore_color(missing_value_path);
    const auto missing_value_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_value_path.string(),
            "--fore-color-object",
            "--fore-color-target-unique-id", "one-guid",
            "--json"
        },
        temp_root);
    expect(missing_value_process.exit_code == 2,
        "#1067: fore-color-object without fore-color should fail during launch parsing");
    expect(visual_object_property(missing_value_path, "one-guid", "FORECOLOR") == "0",
        "#1067: fore-color-object without fore-color should not mutate the asset");

    const fs::path negative_path = temp_root / "negative.scx";
    write_synthetic_form_table_for_object_fore_color(negative_path);
    const auto negative_process = run_process_capture(
        studio_host_path,
        {
            "--path", negative_path.string(),
            "--fore-color-object",
            "--fore-color", "-1",
            "--fore-color-target-unique-id", "one-guid",
            "--json"
        },
        temp_root);
    expect(negative_process.exit_code == 2,
        "#1067: negative fore-color values should fail during launch parsing");
    expect(visual_object_property(negative_path, "one-guid", "FORECOLOR") == "0",
        "#1067: negative fore-color values should not mutate the asset");

    const fs::path duplicate_path = temp_root / "duplicate.scx";
    write_synthetic_form_table_for_object_fore_color(duplicate_path);
    const auto duplicate_process = run_process_capture(
        studio_host_path,
        {
            "--path", duplicate_path.string(),
            "--fore-color-object",
            "--fore-color", "8421504",
            "--fore-color-target-unique-id", "one-guid",
            "--fore-color-target-object-name", "lstCustomers",
            "--json"
        },
        temp_root);
    expect(duplicate_process.exit_code == 4,
        "#1067: duplicate-target host object fore-color assignment should return command failure");
    expect(visual_object_property(duplicate_path, "one-guid", "FORECOLOR") == "0",
        "#1067: duplicate-target host object fore-color assignment should not mutate the asset");

    const fs::path ambiguous_path = temp_root / "ambiguous.scx";
    write_synthetic_form_table_for_object_fore_color(ambiguous_path);
    const auto ambiguous_process = run_process_capture(
        studio_host_path,
        {
            "--path", ambiguous_path.string(),
            "--fore-color-object",
            "--back-color-object",
            "--fore-color", "8421504",
            "--fore-color-target-unique-id", "one-guid",
            "--back-color", "65280",
            "--back-color-target-unique-id", "one-guid",
            "--json"
        },
        temp_root);
    expect(ambiguous_process.exit_code == 2,
        "#1067: fore-color-object plus back-color-object requests should fail during launch parsing");
    expect(visual_object_property(ambiguous_path, "one-guid", "FORECOLOR") == "0",
        "#1067: fore-color-object ambiguity should not mutate the asset");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_assigns_disabled_back_color_by_stable_selectors(const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_disabled_back_color_object_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path disabled_back_color_path = temp_root / "disabled_back_color.scx";
    write_synthetic_form_table_for_object_disabled_back_color(disabled_back_color_path);
    const auto disabled_back_color_process = run_process_capture(
        studio_host_path,
        {
            "--path", disabled_back_color_path.string(),
            "--disabled-back-color-object",
            "--disabled-back-color", "8421504",
            "--disabled-back-color-target-object-name", "lstCustomers",
            "--disabled-back-color-target-unique-id", "two-guid",
            "--json"
        },
        temp_root);
    expect(disabled_back_color_process.exit_code == 0,
        "#1068: host object disabled-back-color assignment should exit successfully");
    expect(visual_object_property(disabled_back_color_path, "one-guid", "DISABLEDBACKCOLOR") == "8421504" &&
            visual_object_property(disabled_back_color_path, "two-guid", "DISABLEDBACKCOLOR") == "8421504" &&
            visual_object_property(disabled_back_color_path, "three-guid", "DISABLEDBACKCOLOR") == "255" &&
            visual_object_property(disabled_back_color_path, "other-guid", "DISABLEDBACKCOLOR") == "65280",
        "#1068: host object disabled-back-color assignment should assign selected numeric values and preserve unrelated objects");

    const fs::path missing_target_path = temp_root / "missing_target.scx";
    write_synthetic_form_table_for_object_disabled_back_color(missing_target_path);
    const auto missing_target_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_target_path.string(),
            "--disabled-back-color-object",
            "--disabled-back-color", "8421504",
            "--disabled-back-color-target-unique-id", "one-guid",
            "--disabled-back-color-target-unique-id", "missing-guid",
            "--json"
        },
        temp_root);
    expect(missing_target_process.exit_code == 4,
        "#1068: missing-target host object disabled-back-color assignment should return command failure");
    expect(visual_object_property(missing_target_path, "one-guid", "DISABLEDBACKCOLOR") == "16777215" &&
            visual_object_property(missing_target_path, "two-guid", "DISABLEDBACKCOLOR") == "12632256",
        "#1068: missing-target host object disabled-back-color assignment should not mutate the asset");

    const fs::path missing_selector_path = temp_root / "missing_selector.scx";
    write_synthetic_form_table_for_object_disabled_back_color(missing_selector_path);
    const auto missing_selector_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_selector_path.string(),
            "--disabled-back-color-object",
            "--disabled-back-color", "8421504",
            "--json"
        },
        temp_root);
    expect(missing_selector_process.exit_code == 2,
        "#1068: disabled-back-color-object without target selectors should fail during launch parsing");
    expect(visual_object_property(missing_selector_path, "one-guid", "DISABLEDBACKCOLOR") == "16777215",
        "#1068: disabled-back-color-object without target selectors should not mutate the asset");

    const fs::path missing_value_path = temp_root / "missing_value.scx";
    write_synthetic_form_table_for_object_disabled_back_color(missing_value_path);
    const auto missing_value_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_value_path.string(),
            "--disabled-back-color-object",
            "--disabled-back-color-target-unique-id", "one-guid",
            "--json"
        },
        temp_root);
    expect(missing_value_process.exit_code == 2,
        "#1068: disabled-back-color-object without disabled-back-color should fail during launch parsing");
    expect(visual_object_property(missing_value_path, "one-guid", "DISABLEDBACKCOLOR") == "16777215",
        "#1068: disabled-back-color-object without disabled-back-color should not mutate the asset");

    const fs::path negative_path = temp_root / "negative.scx";
    write_synthetic_form_table_for_object_disabled_back_color(negative_path);
    const auto negative_process = run_process_capture(
        studio_host_path,
        {
            "--path", negative_path.string(),
            "--disabled-back-color-object",
            "--disabled-back-color", "-1",
            "--disabled-back-color-target-unique-id", "one-guid",
            "--json"
        },
        temp_root);
    expect(negative_process.exit_code == 2,
        "#1068: negative disabled-back-color values should fail during launch parsing");
    expect(visual_object_property(negative_path, "one-guid", "DISABLEDBACKCOLOR") == "16777215",
        "#1068: negative disabled-back-color values should not mutate the asset");

    const fs::path duplicate_path = temp_root / "duplicate.scx";
    write_synthetic_form_table_for_object_disabled_back_color(duplicate_path);
    const auto duplicate_process = run_process_capture(
        studio_host_path,
        {
            "--path", duplicate_path.string(),
            "--disabled-back-color-object",
            "--disabled-back-color", "8421504",
            "--disabled-back-color-target-unique-id", "one-guid",
            "--disabled-back-color-target-object-name", "lstCustomers",
            "--json"
        },
        temp_root);
    expect(duplicate_process.exit_code == 4,
        "#1068: duplicate-target host object disabled-back-color assignment should return command failure");
    expect(visual_object_property(duplicate_path, "one-guid", "DISABLEDBACKCOLOR") == "16777215",
        "#1068: duplicate-target host object disabled-back-color assignment should not mutate the asset");

    const fs::path ambiguous_path = temp_root / "ambiguous.scx";
    write_synthetic_form_table_for_object_disabled_back_color(ambiguous_path);
    const auto ambiguous_process = run_process_capture(
        studio_host_path,
        {
            "--path", ambiguous_path.string(),
            "--disabled-back-color-object",
            "--fore-color-object",
            "--disabled-back-color", "8421504",
            "--disabled-back-color-target-unique-id", "one-guid",
            "--fore-color", "65280",
            "--fore-color-target-unique-id", "one-guid",
            "--json"
        },
        temp_root);
    expect(ambiguous_process.exit_code == 2,
        "#1068: disabled-back-color-object plus fore-color-object requests should fail during launch parsing");
    expect(visual_object_property(ambiguous_path, "one-guid", "DISABLEDBACKCOLOR") == "16777215",
        "#1068: disabled-back-color-object ambiguity should not mutate the asset");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_assigns_disabled_fore_color_by_stable_selectors(const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_disabled_fore_color_object_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path disabled_fore_color_path = temp_root / "disabled_fore_color.scx";
    write_synthetic_form_table_for_object_disabled_fore_color(disabled_fore_color_path);
    const auto disabled_fore_color_process = run_process_capture(
        studio_host_path,
        {
            "--path", disabled_fore_color_path.string(),
            "--disabled-fore-color-object",
            "--disabled-fore-color", "8421504",
            "--disabled-fore-color-target-object-name", "lstCustomers",
            "--disabled-fore-color-target-unique-id", "two-guid",
            "--json"
        },
        temp_root);
    expect(disabled_fore_color_process.exit_code == 0,
        "#1069: host object disabled-fore-color assignment should exit successfully");
    expect(visual_object_property(disabled_fore_color_path, "one-guid", "DISABLEDFORECOLOR") == "8421504" &&
            visual_object_property(disabled_fore_color_path, "two-guid", "DISABLEDFORECOLOR") == "8421504" &&
            visual_object_property(disabled_fore_color_path, "three-guid", "DISABLEDFORECOLOR") == "16777215" &&
            visual_object_property(disabled_fore_color_path, "other-guid", "DISABLEDFORECOLOR") == "65280",
        "#1069: host object disabled-fore-color assignment should assign selected numeric values and preserve unrelated objects");

    const fs::path missing_target_path = temp_root / "missing_target.scx";
    write_synthetic_form_table_for_object_disabled_fore_color(missing_target_path);
    const auto missing_target_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_target_path.string(),
            "--disabled-fore-color-object",
            "--disabled-fore-color", "8421504",
            "--disabled-fore-color-target-unique-id", "one-guid",
            "--disabled-fore-color-target-unique-id", "missing-guid",
            "--json"
        },
        temp_root);
    expect(missing_target_process.exit_code == 4,
        "#1069: missing-target host object disabled-fore-color assignment should return command failure");
    expect(visual_object_property(missing_target_path, "one-guid", "DISABLEDFORECOLOR") == "0" &&
            visual_object_property(missing_target_path, "two-guid", "DISABLEDFORECOLOR") == "255",
        "#1069: missing-target host object disabled-fore-color assignment should not mutate the asset");

    const fs::path missing_selector_path = temp_root / "missing_selector.scx";
    write_synthetic_form_table_for_object_disabled_fore_color(missing_selector_path);
    const auto missing_selector_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_selector_path.string(),
            "--disabled-fore-color-object",
            "--disabled-fore-color", "8421504",
            "--json"
        },
        temp_root);
    expect(missing_selector_process.exit_code == 2,
        "#1069: disabled-fore-color-object without target selectors should fail during launch parsing");
    expect(visual_object_property(missing_selector_path, "one-guid", "DISABLEDFORECOLOR") == "0",
        "#1069: disabled-fore-color-object without target selectors should not mutate the asset");

    const fs::path missing_value_path = temp_root / "missing_value.scx";
    write_synthetic_form_table_for_object_disabled_fore_color(missing_value_path);
    const auto missing_value_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_value_path.string(),
            "--disabled-fore-color-object",
            "--disabled-fore-color-target-unique-id", "one-guid",
            "--json"
        },
        temp_root);
    expect(missing_value_process.exit_code == 2,
        "#1069: disabled-fore-color-object without disabled-fore-color should fail during launch parsing");
    expect(visual_object_property(missing_value_path, "one-guid", "DISABLEDFORECOLOR") == "0",
        "#1069: disabled-fore-color-object without disabled-fore-color should not mutate the asset");

    const fs::path negative_path = temp_root / "negative.scx";
    write_synthetic_form_table_for_object_disabled_fore_color(negative_path);
    const auto negative_process = run_process_capture(
        studio_host_path,
        {
            "--path", negative_path.string(),
            "--disabled-fore-color-object",
            "--disabled-fore-color", "-1",
            "--disabled-fore-color-target-unique-id", "one-guid",
            "--json"
        },
        temp_root);
    expect(negative_process.exit_code == 2,
        "#1069: negative disabled-fore-color values should fail during launch parsing");
    expect(visual_object_property(negative_path, "one-guid", "DISABLEDFORECOLOR") == "0",
        "#1069: negative disabled-fore-color values should not mutate the asset");

    const fs::path duplicate_path = temp_root / "duplicate.scx";
    write_synthetic_form_table_for_object_disabled_fore_color(duplicate_path);
    const auto duplicate_process = run_process_capture(
        studio_host_path,
        {
            "--path", duplicate_path.string(),
            "--disabled-fore-color-object",
            "--disabled-fore-color", "8421504",
            "--disabled-fore-color-target-unique-id", "one-guid",
            "--disabled-fore-color-target-object-name", "lstCustomers",
            "--json"
        },
        temp_root);
    expect(duplicate_process.exit_code == 4,
        "#1069: duplicate-target host object disabled-fore-color assignment should return command failure");
    expect(visual_object_property(duplicate_path, "one-guid", "DISABLEDFORECOLOR") == "0",
        "#1069: duplicate-target host object disabled-fore-color assignment should not mutate the asset");

    const fs::path ambiguous_path = temp_root / "ambiguous.scx";
    write_synthetic_form_table_for_object_disabled_fore_color(ambiguous_path);
    const auto ambiguous_process = run_process_capture(
        studio_host_path,
        {
            "--path", ambiguous_path.string(),
            "--disabled-fore-color-object",
            "--disabled-back-color-object",
            "--disabled-fore-color", "8421504",
            "--disabled-fore-color-target-unique-id", "one-guid",
            "--disabled-back-color", "65280",
            "--disabled-back-color-target-unique-id", "one-guid",
            "--json"
        },
        temp_root);
    expect(ambiguous_process.exit_code == 2,
        "#1069: disabled-fore-color-object plus disabled-back-color-object requests should fail during launch parsing");
    expect(visual_object_property(ambiguous_path, "one-guid", "DISABLEDFORECOLOR") == "0",
        "#1069: disabled-fore-color-object ambiguity should not mutate the asset");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_assigns_dynamic_back_color_by_stable_selectors(const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_dynamic_back_color_object_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path dynamic_back_color_path = temp_root / "dynamic_back_color.scx";
    write_synthetic_form_table_for_object_dynamic_back_color(dynamic_back_color_path);
    const auto dynamic_back_color_process = run_process_capture(
        studio_host_path,
        {
            "--path", dynamic_back_color_path.string(),
            "--dynamic-back-color-object",
            "--dynamic-back-color", "IIF(.T., RGB(1,2,3), RGB(4,5,6))",
            "--dynamic-back-color-target-object-name", "lstCustomers",
            "--dynamic-back-color-target-unique-id", "two-guid",
            "--json"
        },
        temp_root);
    expect(dynamic_back_color_process.exit_code == 0,
        "#1070: host object dynamic-back-color assignment should exit successfully");
    expect(visual_object_property(dynamic_back_color_path, "one-guid", "DYNAMICBACKCOLOR") ==
                "IIF(.T., RGB(1,2,3), RGB(4,5,6))" &&
            visual_object_property(dynamic_back_color_path, "two-guid", "DYNAMICBACKCOLOR") ==
                "IIF(.T., RGB(1,2,3), RGB(4,5,6))" &&
            visual_object_property(dynamic_back_color_path, "three-guid", "DYNAMICBACKCOLOR") == "RGB(2,2,2)" &&
            visual_object_property(dynamic_back_color_path, "other-guid", "DYNAMICBACKCOLOR") == "RGB(3,3,3)",
        "#1070: host object dynamic-back-color assignment should preserve raw expressions and unrelated objects");

    const fs::path missing_target_path = temp_root / "missing_target.scx";
    write_synthetic_form_table_for_object_dynamic_back_color(missing_target_path);
    const auto missing_target_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_target_path.string(),
            "--dynamic-back-color-object",
            "--dynamic-back-color", "RGB(9,9,9)",
            "--dynamic-back-color-target-unique-id", "one-guid",
            "--dynamic-back-color-target-unique-id", "missing-guid",
            "--json"
        },
        temp_root);
    expect(missing_target_process.exit_code == 4,
        "#1070: missing-target host object dynamic-back-color assignment should return command failure");
    expect(visual_object_property(missing_target_path, "one-guid", "DYNAMICBACKCOLOR") == "RGB(0,0,0)" &&
            visual_object_property(missing_target_path, "two-guid", "DYNAMICBACKCOLOR") == "RGB(1,1,1)",
        "#1070: missing-target host object dynamic-back-color assignment should not mutate the asset");

    const fs::path missing_selector_path = temp_root / "missing_selector.scx";
    write_synthetic_form_table_for_object_dynamic_back_color(missing_selector_path);
    const auto missing_selector_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_selector_path.string(),
            "--dynamic-back-color-object",
            "--dynamic-back-color", "RGB(9,9,9)",
            "--json"
        },
        temp_root);
    expect(missing_selector_process.exit_code == 2,
        "#1070: dynamic-back-color-object without target selectors should fail during launch parsing");
    expect(visual_object_property(missing_selector_path, "one-guid", "DYNAMICBACKCOLOR") == "RGB(0,0,0)",
        "#1070: dynamic-back-color-object without target selectors should not mutate the asset");

    const fs::path missing_value_path = temp_root / "missing_value.scx";
    write_synthetic_form_table_for_object_dynamic_back_color(missing_value_path);
    const auto missing_value_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_value_path.string(),
            "--dynamic-back-color-object",
            "--dynamic-back-color-target-unique-id", "one-guid",
            "--json"
        },
        temp_root);
    expect(missing_value_process.exit_code == 2,
        "#1070: dynamic-back-color-object without dynamic-back-color should fail during launch parsing");
    expect(visual_object_property(missing_value_path, "one-guid", "DYNAMICBACKCOLOR") == "RGB(0,0,0)",
        "#1070: dynamic-back-color-object without dynamic-back-color should not mutate the asset");

    const fs::path duplicate_path = temp_root / "duplicate.scx";
    write_synthetic_form_table_for_object_dynamic_back_color(duplicate_path);
    const auto duplicate_process = run_process_capture(
        studio_host_path,
        {
            "--path", duplicate_path.string(),
            "--dynamic-back-color-object",
            "--dynamic-back-color", "RGB(9,9,9)",
            "--dynamic-back-color-target-unique-id", "one-guid",
            "--dynamic-back-color-target-object-name", "lstCustomers",
            "--json"
        },
        temp_root);
    expect(duplicate_process.exit_code == 4,
        "#1070: duplicate-target host object dynamic-back-color assignment should return command failure");
    expect(visual_object_property(duplicate_path, "one-guid", "DYNAMICBACKCOLOR") == "RGB(0,0,0)",
        "#1070: duplicate-target host object dynamic-back-color assignment should not mutate the asset");

    const fs::path ambiguous_path = temp_root / "ambiguous.scx";
    write_synthetic_form_table_for_object_dynamic_back_color(ambiguous_path);
    const auto ambiguous_process = run_process_capture(
        studio_host_path,
        {
            "--path", ambiguous_path.string(),
            "--dynamic-back-color-object",
            "--disabled-fore-color-object",
            "--dynamic-back-color", "RGB(9,9,9)",
            "--dynamic-back-color-target-unique-id", "one-guid",
            "--disabled-fore-color", "65280",
            "--disabled-fore-color-target-unique-id", "one-guid",
            "--json"
        },
        temp_root);
    expect(ambiguous_process.exit_code == 2,
        "#1070: dynamic-back-color-object plus disabled-fore-color-object requests should fail during launch parsing");
    expect(visual_object_property(ambiguous_path, "one-guid", "DYNAMICBACKCOLOR") == "RGB(0,0,0)",
        "#1070: dynamic-back-color-object ambiguity should not mutate the asset");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_assigns_dynamic_fore_color_by_stable_selectors(const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_dynamic_fore_color_object_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path dynamic_fore_color_path = temp_root / "dynamic_fore_color.scx";
    write_synthetic_form_table_for_object_dynamic_fore_color(dynamic_fore_color_path);
    const auto dynamic_fore_color_process = run_process_capture(
        studio_host_path,
        {
            "--path", dynamic_fore_color_path.string(),
            "--dynamic-fore-color-object",
            "--dynamic-fore-color", "IIF(.T., RGB(7,8,9), RGB(4,5,6))",
            "--dynamic-fore-color-target-object-name", "lstCustomers",
            "--dynamic-fore-color-target-unique-id", "two-guid",
            "--json"
        },
        temp_root);
    expect(dynamic_fore_color_process.exit_code == 0,
        "#1071: host object dynamic-fore-color assignment should exit successfully");
    expect(visual_object_property(dynamic_fore_color_path, "one-guid", "DYNAMICFORECOLOR") ==
                "IIF(.T., RGB(7,8,9), RGB(4,5,6))" &&
            visual_object_property(dynamic_fore_color_path, "two-guid", "DYNAMICFORECOLOR") ==
                "IIF(.T., RGB(7,8,9), RGB(4,5,6))" &&
            visual_object_property(dynamic_fore_color_path, "three-guid", "DYNAMICFORECOLOR") == "RGB(2,2,2)" &&
            visual_object_property(dynamic_fore_color_path, "other-guid", "DYNAMICFORECOLOR") == "RGB(3,3,3)",
        "#1071: host object dynamic-fore-color assignment should preserve raw expressions and unrelated objects");

    const fs::path missing_target_path = temp_root / "missing_target.scx";
    write_synthetic_form_table_for_object_dynamic_fore_color(missing_target_path);
    const auto missing_target_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_target_path.string(),
            "--dynamic-fore-color-object",
            "--dynamic-fore-color", "RGB(9,9,9)",
            "--dynamic-fore-color-target-unique-id", "one-guid",
            "--dynamic-fore-color-target-unique-id", "missing-guid",
            "--json"
        },
        temp_root);
    expect(missing_target_process.exit_code == 4,
        "#1071: missing-target host object dynamic-fore-color assignment should return command failure");
    expect(visual_object_property(missing_target_path, "one-guid", "DYNAMICFORECOLOR") == "RGB(0,0,0)" &&
            visual_object_property(missing_target_path, "two-guid", "DYNAMICFORECOLOR") == "RGB(1,1,1)",
        "#1071: missing-target host object dynamic-fore-color assignment should not mutate the asset");

    const fs::path missing_selector_path = temp_root / "missing_selector.scx";
    write_synthetic_form_table_for_object_dynamic_fore_color(missing_selector_path);
    const auto missing_selector_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_selector_path.string(),
            "--dynamic-fore-color-object",
            "--dynamic-fore-color", "RGB(9,9,9)",
            "--json"
        },
        temp_root);
    expect(missing_selector_process.exit_code == 2,
        "#1071: dynamic-fore-color-object without target selectors should fail during launch parsing");
    expect(visual_object_property(missing_selector_path, "one-guid", "DYNAMICFORECOLOR") == "RGB(0,0,0)",
        "#1071: dynamic-fore-color-object without target selectors should not mutate the asset");

    const fs::path missing_value_path = temp_root / "missing_value.scx";
    write_synthetic_form_table_for_object_dynamic_fore_color(missing_value_path);
    const auto missing_value_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_value_path.string(),
            "--dynamic-fore-color-object",
            "--dynamic-fore-color-target-unique-id", "one-guid",
            "--json"
        },
        temp_root);
    expect(missing_value_process.exit_code == 2,
        "#1071: dynamic-fore-color-object without dynamic-fore-color should fail during launch parsing");
    expect(visual_object_property(missing_value_path, "one-guid", "DYNAMICFORECOLOR") == "RGB(0,0,0)",
        "#1071: dynamic-fore-color-object without dynamic-fore-color should not mutate the asset");

    const fs::path duplicate_path = temp_root / "duplicate.scx";
    write_synthetic_form_table_for_object_dynamic_fore_color(duplicate_path);
    const auto duplicate_process = run_process_capture(
        studio_host_path,
        {
            "--path", duplicate_path.string(),
            "--dynamic-fore-color-object",
            "--dynamic-fore-color", "RGB(9,9,9)",
            "--dynamic-fore-color-target-unique-id", "one-guid",
            "--dynamic-fore-color-target-object-name", "lstCustomers",
            "--json"
        },
        temp_root);
    expect(duplicate_process.exit_code == 4,
        "#1071: duplicate-target host object dynamic-fore-color assignment should return command failure");
    expect(visual_object_property(duplicate_path, "one-guid", "DYNAMICFORECOLOR") == "RGB(0,0,0)",
        "#1071: duplicate-target host object dynamic-fore-color assignment should not mutate the asset");

    const fs::path ambiguous_path = temp_root / "ambiguous.scx";
    write_synthetic_form_table_for_object_dynamic_fore_color(ambiguous_path);
    const auto ambiguous_process = run_process_capture(
        studio_host_path,
        {
            "--path", ambiguous_path.string(),
            "--dynamic-fore-color-object",
            "--dynamic-back-color-object",
            "--dynamic-fore-color", "RGB(9,9,9)",
            "--dynamic-fore-color-target-unique-id", "one-guid",
            "--dynamic-back-color", "RGB(1,2,3)",
            "--dynamic-back-color-target-unique-id", "one-guid",
            "--json"
        },
        temp_root);
    expect(ambiguous_process.exit_code == 2,
        "#1071: dynamic-fore-color-object plus dynamic-back-color-object requests should fail during launch parsing");
    expect(visual_object_property(ambiguous_path, "one-guid", "DYNAMICFORECOLOR") == "RGB(0,0,0)",
        "#1071: dynamic-fore-color-object ambiguity should not mutate the asset");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

}  // namespace cf_test_studio_host_json
