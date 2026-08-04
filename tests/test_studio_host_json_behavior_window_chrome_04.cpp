// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only

#include "test_studio_host_json_support.h"

namespace cf_test_studio_host_json {
void write_synthetic_form_table_for_object_max_height(const std::filesystem::path& form_path) {
    const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
        {.name = "OBJNAME", .type = 'C', .length = 24U},
        {.name = "NAME", .type = 'C', .length = 24U},
        {.name = "UNIQUEID", .type = 'C', .length = 32U},
        {.name = "MAXHEIGHT", .type = 'N', .length = 10U, .decimal_count = 0U}
    };
    const std::vector<std::vector<std::string>> records{
        {"frmCustomer", "frmCustomer", "one-guid", "100"},
        {"frmOrder", "frmOrder", "two-guid", "200"},
        {"cntDetails", "cntDetails", "three-guid", "300"},
        {"frmOther", "frmOther", "other-guid", "400"}
    };

    const auto create_result = copperfin::vfp::create_dbf_table_file(form_path.string(), fields, records);
    expect(create_result.ok, "#1151: synthetic SCX table for object max height should be created");
}

void test_studio_host_json_assigns_movable_by_stable_selectors(const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root = fs::temp_directory_path() / "copperfin_studio_host_movable_object_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path movable_path = temp_root / "movable.scx";
    write_synthetic_form_table_for_object_movable(movable_path);
    const auto movable_process = run_process_capture(
        studio_host_path,
        {
            "--path", movable_path.string(),
            "--movable-object",
            "--movable", "false",
            "--movable-target-object-name", "frmCustomer",
            "--movable-target-unique-id", "two-guid",
            "--json"
        },
        temp_root);
    expect(movable_process.exit_code == 0,
        "#1158: host object movable assignment should exit successfully");
    expect(visual_object_property(movable_path, "one-guid", "MOVABLE") == "false" &&
            visual_object_property(movable_path, "two-guid", "MOVABLE") == "false" &&
            visual_object_property(movable_path, "three-guid", "MOVABLE") == "false" &&
            visual_object_property(movable_path, "other-guid", "MOVABLE") == "true",
        "#1158: host object movable assignment should assign selected logical state and preserve unrelated objects");

    const fs::path missing_target_path = temp_root / "missing_target.scx";
    write_synthetic_form_table_for_object_movable(missing_target_path);
    const auto missing_target_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_target_path.string(),
            "--movable-object",
            "--movable", "false",
            "--movable-target-unique-id", "one-guid",
            "--movable-target-unique-id", "missing-guid",
            "--json"
        },
        temp_root);
    expect(missing_target_process.exit_code == 4,
        "#1158: missing-target host object movable assignment should return command failure");
    expect(visual_object_property(missing_target_path, "one-guid", "MOVABLE") == "true" &&
            visual_object_property(missing_target_path, "two-guid", "MOVABLE") == "true",
        "#1158: missing-target host object movable assignment should not mutate the asset");

    const fs::path missing_selector_path = temp_root / "missing_selector.scx";
    write_synthetic_form_table_for_object_movable(missing_selector_path);
    const auto missing_selector_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_selector_path.string(),
            "--movable-object",
            "--movable", "false",
            "--json"
        },
        temp_root);
    expect(missing_selector_process.exit_code == 2,
        "#1158: movable-object without target selectors should fail during launch parsing");
    expect(visual_object_property(missing_selector_path, "one-guid", "MOVABLE") == "true",
        "#1158: movable-object without target selectors should not mutate the asset");

    const fs::path missing_value_path = temp_root / "missing_value.scx";
    write_synthetic_form_table_for_object_movable(missing_value_path);
    const auto missing_value_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_value_path.string(),
            "--movable-object",
            "--movable-target-unique-id", "one-guid",
            "--json"
        },
        temp_root);
    expect(missing_value_process.exit_code == 2,
        "#1158: movable-object without movable value should fail during launch parsing");
    expect(visual_object_property(missing_value_path, "one-guid", "MOVABLE") == "true",
        "#1158: movable-object without movable value should not mutate the asset");

    const fs::path duplicate_path = temp_root / "duplicate.scx";
    write_synthetic_form_table_for_object_movable(duplicate_path);
    const auto duplicate_process = run_process_capture(
        studio_host_path,
        {
            "--path", duplicate_path.string(),
            "--movable-object",
            "--movable", "false",
            "--movable-target-unique-id", "one-guid",
            "--movable-target-object-name", "frmCustomer",
            "--json"
        },
        temp_root);
    expect(duplicate_process.exit_code == 4,
        "#1158: duplicate-target host object movable assignment should return command failure");
    expect(visual_object_property(duplicate_path, "one-guid", "MOVABLE") == "true",
        "#1158: duplicate-target host object movable assignment should not mutate the asset");

    const fs::path ambiguous_path = temp_root / "ambiguous.scx";
    write_synthetic_form_table_for_object_movable(ambiguous_path);
    const auto ambiguous_process = run_process_capture(
        studio_host_path,
        {
            "--path", ambiguous_path.string(),
            "--movable-object",
            "--allow-output-object",
            "--movable", "false",
            "--movable-target-unique-id", "one-guid",
            "--allow-output", "false",
            "--allow-output-target-unique-id", "one-guid",
            "--json"
        },
        temp_root);
    expect(ambiguous_process.exit_code == 2,
        "#1158: movable-object plus allow-output-object requests should fail during launch parsing");
    expect(visual_object_property(ambiguous_path, "one-guid", "MOVABLE") == "true",
        "#1158: movable-object ambiguity should not mutate the asset");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void write_synthetic_form_table_for_object_movable(const std::filesystem::path& form_path) {
    const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
        {.name = "OBJNAME", .type = 'C', .length = 24U},
        {.name = "NAME", .type = 'C', .length = 24U},
        {.name = "UNIQUEID", .type = 'C', .length = 32U},
        {.name = "MOVABLE", .type = 'L', .length = 1U}
    };
    const std::vector<std::vector<std::string>> records{
        {"frmCustomer", "frmCustomer", "one-guid", ".T."},
        {"frmOrder", "frmOrder", "two-guid", ".T."},
        {"cntDetails", "cntDetails", "three-guid", ".F."},
        {"frmOther", "frmOther", "other-guid", ".T."}
    };

    const auto create_result = copperfin::vfp::create_dbf_table_file(form_path.string(), fields, records);
    expect(create_result.ok, "#1158: synthetic SCX table for object movable should be created");
}

void test_studio_host_json_assigns_half_height_caption_by_stable_selectors(const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root = fs::temp_directory_path() / "copperfin_studio_host_half_height_caption_object_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path half_height_caption_path = temp_root / "half_height_caption.scx";
    write_synthetic_form_table_for_object_half_height_caption(half_height_caption_path);
    const auto half_height_caption_process = run_process_capture(
        studio_host_path,
        {
            "--path", half_height_caption_path.string(),
            "--half-height-caption-object",
            "--half-height-caption", "false",
            "--half-height-caption-target-object-name", "frmCustomer",
            "--half-height-caption-target-unique-id", "two-guid",
            "--json"
        },
        temp_root);
    expect(half_height_caption_process.exit_code == 0,
        "#1159: host object half-height-caption assignment should exit successfully");
    expect(visual_object_property(half_height_caption_path, "one-guid", "HALFHEIGHTCAPTION") == "false" &&
            visual_object_property(half_height_caption_path, "two-guid", "HALFHEIGHTCAPTION") == "false" &&
            visual_object_property(half_height_caption_path, "three-guid", "HALFHEIGHTCAPTION") == "false" &&
            visual_object_property(half_height_caption_path, "other-guid", "HALFHEIGHTCAPTION") == "true",
        "#1159: host object half-height-caption assignment should assign selected logical state and preserve unrelated objects");

    const fs::path missing_target_path = temp_root / "missing_target.scx";
    write_synthetic_form_table_for_object_half_height_caption(missing_target_path);
    const auto missing_target_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_target_path.string(),
            "--half-height-caption-object",
            "--half-height-caption", "false",
            "--half-height-caption-target-unique-id", "one-guid",
            "--half-height-caption-target-unique-id", "missing-guid",
            "--json"
        },
        temp_root);
    expect(missing_target_process.exit_code == 4,
        "#1159: missing-target host object half-height-caption assignment should return command failure");
    expect(visual_object_property(missing_target_path, "one-guid", "HALFHEIGHTCAPTION") == "true" &&
            visual_object_property(missing_target_path, "two-guid", "HALFHEIGHTCAPTION") == "true",
        "#1159: missing-target host object half-height-caption assignment should not mutate the asset");

    const fs::path missing_selector_path = temp_root / "missing_selector.scx";
    write_synthetic_form_table_for_object_half_height_caption(missing_selector_path);
    const auto missing_selector_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_selector_path.string(),
            "--half-height-caption-object",
            "--half-height-caption", "false",
            "--json"
        },
        temp_root);
    expect(missing_selector_process.exit_code == 2,
        "#1159: half-height-caption-object without target selectors should fail during launch parsing");
    expect(visual_object_property(missing_selector_path, "one-guid", "HALFHEIGHTCAPTION") == "true",
        "#1159: half-height-caption-object without target selectors should not mutate the asset");

    const fs::path missing_value_path = temp_root / "missing_value.scx";
    write_synthetic_form_table_for_object_half_height_caption(missing_value_path);
    const auto missing_value_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_value_path.string(),
            "--half-height-caption-object",
            "--half-height-caption-target-unique-id", "one-guid",
            "--json"
        },
        temp_root);
    expect(missing_value_process.exit_code == 2,
        "#1159: half-height-caption-object without half-height-caption value should fail during launch parsing");
    expect(visual_object_property(missing_value_path, "one-guid", "HALFHEIGHTCAPTION") == "true",
        "#1159: half-height-caption-object without half-height-caption value should not mutate the asset");

    const fs::path duplicate_path = temp_root / "duplicate.scx";
    write_synthetic_form_table_for_object_half_height_caption(duplicate_path);
    const auto duplicate_process = run_process_capture(
        studio_host_path,
        {
            "--path", duplicate_path.string(),
            "--half-height-caption-object",
            "--half-height-caption", "false",
            "--half-height-caption-target-unique-id", "one-guid",
            "--half-height-caption-target-object-name", "frmCustomer",
            "--json"
        },
        temp_root);
    expect(duplicate_process.exit_code == 4,
        "#1159: duplicate-target host object half-height-caption assignment should return command failure");
    expect(visual_object_property(duplicate_path, "one-guid", "HALFHEIGHTCAPTION") == "true",
        "#1159: duplicate-target host object half-height-caption assignment should not mutate the asset");

    const fs::path ambiguous_path = temp_root / "ambiguous.scx";
    write_synthetic_form_table_for_object_half_height_caption(ambiguous_path);
    const auto ambiguous_process = run_process_capture(
        studio_host_path,
        {
            "--path", ambiguous_path.string(),
            "--half-height-caption-object",
            "--allow-output-object",
            "--half-height-caption", "false",
            "--half-height-caption-target-unique-id", "one-guid",
            "--allow-output", "false",
            "--allow-output-target-unique-id", "one-guid",
            "--json"
        },
        temp_root);
    expect(ambiguous_process.exit_code == 2,
        "#1159: half-height-caption-object plus allow-output-object requests should fail during launch parsing");
    expect(visual_object_property(ambiguous_path, "one-guid", "HALFHEIGHTCAPTION") == "true",
        "#1159: half-height-caption-object ambiguity should not mutate the asset");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void write_synthetic_form_table_for_object_half_height_caption(const std::filesystem::path& form_path) {
    const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
        {.name = "OBJNAME", .type = 'C', .length = 24U},
        {.name = "NAME", .type = 'C', .length = 24U},
        {.name = "UNIQUEID", .type = 'C', .length = 32U},
        {.name = "HALFHEIGHT", .type = 'L', .length = 1U}
    };
    const std::vector<std::vector<std::string>> records{
        {"frmCustomer", "frmCustomer", "one-guid", ".T."},
        {"frmOrder", "frmOrder", "two-guid", ".T."},
        {"cntDetails", "cntDetails", "three-guid", ".F."},
        {"frmOther", "frmOther", "other-guid", ".T."}
    };

    const auto create_result = copperfin::vfp::create_dbf_table_file(form_path.string(), fields, records);
    expect(create_result.ok, "#1159: synthetic SCX table for object half-height-caption should be created");
}

void test_studio_host_json_assigns_mdi_form_by_stable_selectors(const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root = fs::temp_directory_path() / "copperfin_studio_host_mdi_form_object_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path mdi_form_path = temp_root / "mdi_form.scx";
    write_synthetic_form_table_for_object_mdi_form(mdi_form_path);
    const auto mdi_form_process = run_process_capture(
        studio_host_path,
        {
            "--path", mdi_form_path.string(),
            "--mdi-form-object",
            "--mdi-form", "false",
            "--mdi-form-target-object-name", "frmCustomer",
            "--mdi-form-target-unique-id", "two-guid",
            "--json"
        },
        temp_root);
    expect(mdi_form_process.exit_code == 0,
        "#1160: host object MDI-form assignment should exit successfully");
    expect(visual_object_property(mdi_form_path, "one-guid", "MDIFORM") == "false" &&
            visual_object_property(mdi_form_path, "two-guid", "MDIFORM") == "false" &&
            visual_object_property(mdi_form_path, "three-guid", "MDIFORM") == "false" &&
            visual_object_property(mdi_form_path, "other-guid", "MDIFORM") == "true",
        "#1160: host object MDI-form assignment should assign selected logical state and preserve unrelated objects");

    const fs::path missing_target_path = temp_root / "missing_target.scx";
    write_synthetic_form_table_for_object_mdi_form(missing_target_path);
    const auto missing_target_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_target_path.string(),
            "--mdi-form-object",
            "--mdi-form", "false",
            "--mdi-form-target-unique-id", "one-guid",
            "--mdi-form-target-unique-id", "missing-guid",
            "--json"
        },
        temp_root);
    expect(missing_target_process.exit_code == 4,
        "#1160: missing-target host object MDI-form assignment should return command failure");
    expect(visual_object_property(missing_target_path, "one-guid", "MDIFORM") == "true" &&
            visual_object_property(missing_target_path, "two-guid", "MDIFORM") == "true",
        "#1160: missing-target host object MDI-form assignment should not mutate the asset");

    const fs::path missing_selector_path = temp_root / "missing_selector.scx";
    write_synthetic_form_table_for_object_mdi_form(missing_selector_path);
    const auto missing_selector_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_selector_path.string(),
            "--mdi-form-object",
            "--mdi-form", "false",
            "--json"
        },
        temp_root);
    expect(missing_selector_process.exit_code == 2,
        "#1160: mdi-form-object without target selectors should fail during launch parsing");
    expect(visual_object_property(missing_selector_path, "one-guid", "MDIFORM") == "true",
        "#1160: mdi-form-object without target selectors should not mutate the asset");

    const fs::path missing_value_path = temp_root / "missing_value.scx";
    write_synthetic_form_table_for_object_mdi_form(missing_value_path);
    const auto missing_value_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_value_path.string(),
            "--mdi-form-object",
            "--mdi-form-target-unique-id", "one-guid",
            "--json"
        },
        temp_root);
    expect(missing_value_process.exit_code == 2,
        "#1160: mdi-form-object without MDI-form value should fail during launch parsing");
    expect(visual_object_property(missing_value_path, "one-guid", "MDIFORM") == "true",
        "#1160: mdi-form-object without MDI-form value should not mutate the asset");

    const fs::path duplicate_path = temp_root / "duplicate.scx";
    write_synthetic_form_table_for_object_mdi_form(duplicate_path);
    const auto duplicate_process = run_process_capture(
        studio_host_path,
        {
            "--path", duplicate_path.string(),
            "--mdi-form-object",
            "--mdi-form", "false",
            "--mdi-form-target-unique-id", "one-guid",
            "--mdi-form-target-object-name", "frmCustomer",
            "--json"
        },
        temp_root);
    expect(duplicate_process.exit_code == 4,
        "#1160: duplicate-target host object MDI-form assignment should return command failure");
    expect(visual_object_property(duplicate_path, "one-guid", "MDIFORM") == "true",
        "#1160: duplicate-target host object MDI-form assignment should not mutate the asset");

    const fs::path ambiguous_path = temp_root / "ambiguous.scx";
    write_synthetic_form_table_for_object_mdi_form(ambiguous_path);
    const auto ambiguous_process = run_process_capture(
        studio_host_path,
        {
            "--path", ambiguous_path.string(),
            "--mdi-form-object",
            "--allow-output-object",
            "--mdi-form", "false",
            "--mdi-form-target-unique-id", "one-guid",
            "--allow-output", "false",
            "--allow-output-target-unique-id", "one-guid",
            "--json"
        },
        temp_root);
    expect(ambiguous_process.exit_code == 2,
        "#1160: mdi-form-object plus allow-output-object requests should fail during launch parsing");
    expect(visual_object_property(ambiguous_path, "one-guid", "MDIFORM") == "true",
        "#1160: mdi-form-object ambiguity should not mutate the asset");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void write_synthetic_form_table_for_object_mdi_form(const std::filesystem::path& form_path) {
    const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
        {.name = "OBJNAME", .type = 'C', .length = 24U},
        {.name = "NAME", .type = 'C', .length = 24U},
        {.name = "UNIQUEID", .type = 'C', .length = 32U},
        {.name = "MDIFORM", .type = 'L', .length = 1U}
    };
    const std::vector<std::vector<std::string>> records{
        {"frmCustomer", "frmCustomer", "one-guid", ".T."},
        {"frmOrder", "frmOrder", "two-guid", ".T."},
        {"cntDetails", "cntDetails", "three-guid", ".F."},
        {"frmOther", "frmOther", "other-guid", ".T."}
    };

    const auto create_result = copperfin::vfp::create_dbf_table_file(form_path.string(), fields, records);
    expect(create_result.ok, "#1160: synthetic SCX table for object MDI form should be created");
}

void test_studio_host_json_assigns_max_width_by_stable_selectors(const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root = fs::temp_directory_path() / "copperfin_studio_host_max_width_object_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path max_width_path = temp_root / "max_width.scx";
    write_synthetic_form_table_for_object_max_width(max_width_path);
    const auto max_width_process = run_process_capture(
        studio_host_path,
        {
            "--path", max_width_path.string(),
            "--max-width-object",
            "--max-width", "640",
            "--max-width-target-object-name", "frmCustomer",
            "--max-width-target-unique-id", "two-guid",
            "--json"
        },
        temp_root);
    expect(max_width_process.exit_code == 0,
        "#1152: host object max-width assignment should exit successfully");
    expect(visual_object_property(max_width_path, "one-guid", "MAXWIDTH") == "640" &&
            visual_object_property(max_width_path, "two-guid", "MAXWIDTH") == "640" &&
            visual_object_property(max_width_path, "three-guid", "MAXWIDTH") == "300" &&
            visual_object_property(max_width_path, "other-guid", "MAXWIDTH") == "400",
        "#1152: host object max-width assignment should assign selected numeric value and preserve unrelated objects");

    const fs::path missing_target_path = temp_root / "missing_target.scx";
    write_synthetic_form_table_for_object_max_width(missing_target_path);
    const auto missing_target_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_target_path.string(),
            "--max-width-object",
            "--max-width", "640",
            "--max-width-target-unique-id", "one-guid",
            "--max-width-target-unique-id", "missing-guid",
            "--json"
        },
        temp_root);
    expect(missing_target_process.exit_code == 4,
        "#1152: missing-target host object max-width assignment should return command failure");
    expect(visual_object_property(missing_target_path, "one-guid", "MAXWIDTH") == "100" &&
            visual_object_property(missing_target_path, "two-guid", "MAXWIDTH") == "200",
        "#1152: missing-target host object max-width assignment should not mutate the asset");

    const fs::path missing_selector_path = temp_root / "missing_selector.scx";
    write_synthetic_form_table_for_object_max_width(missing_selector_path);
    const auto missing_selector_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_selector_path.string(),
            "--max-width-object",
            "--max-width", "640",
            "--json"
        },
        temp_root);
    expect(missing_selector_process.exit_code == 2,
        "#1152: max-width-object without target selectors should fail during launch parsing");
    expect(visual_object_property(missing_selector_path, "one-guid", "MAXWIDTH") == "100",
        "#1152: max-width-object without target selectors should not mutate the asset");

    const fs::path missing_value_path = temp_root / "missing_value.scx";
    write_synthetic_form_table_for_object_max_width(missing_value_path);
    const auto missing_value_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_value_path.string(),
            "--max-width-object",
            "--max-width-target-unique-id", "one-guid",
            "--json"
        },
        temp_root);
    expect(missing_value_process.exit_code == 2,
        "#1152: max-width-object without max-width value should fail during launch parsing");
    expect(visual_object_property(missing_value_path, "one-guid", "MAXWIDTH") == "100",
        "#1152: max-width-object without max-width value should not mutate the asset");

    const fs::path negative_value_path = temp_root / "negative_value.scx";
    write_synthetic_form_table_for_object_max_width(negative_value_path);
    const auto negative_value_process = run_process_capture(
        studio_host_path,
        {
            "--path", negative_value_path.string(),
            "--max-width-object",
            "--max-width", "-1",
            "--max-width-target-unique-id", "one-guid",
            "--json"
        },
        temp_root);
    expect(negative_value_process.exit_code == 2,
        "#1152: max-width-object with negative value should fail during launch parsing");
    expect(visual_object_property(negative_value_path, "one-guid", "MAXWIDTH") == "100",
        "#1152: max-width-object with negative value should not mutate the asset");

    const fs::path duplicate_path = temp_root / "duplicate.scx";
    write_synthetic_form_table_for_object_max_width(duplicate_path);
    const auto duplicate_process = run_process_capture(
        studio_host_path,
        {
            "--path", duplicate_path.string(),
            "--max-width-object",
            "--max-width", "640",
            "--max-width-target-unique-id", "one-guid",
            "--max-width-target-object-name", "frmCustomer",
            "--json"
        },
        temp_root);
    expect(duplicate_process.exit_code == 4,
        "#1152: duplicate-target host object max-width assignment should return command failure");
    expect(visual_object_property(duplicate_path, "one-guid", "MAXWIDTH") == "100",
        "#1152: duplicate-target host object max-width assignment should not mutate the asset");

    const fs::path ambiguous_path = temp_root / "ambiguous.scx";
    write_synthetic_form_table_for_object_max_width(ambiguous_path);
    const auto ambiguous_process = run_process_capture(
        studio_host_path,
        {
            "--path", ambiguous_path.string(),
            "--max-width-object",
            "--allow-output-object",
            "--max-width", "640",
            "--max-width-target-unique-id", "one-guid",
            "--allow-output", "false",
            "--allow-output-target-unique-id", "one-guid",
            "--json"
        },
        temp_root);
    expect(ambiguous_process.exit_code == 2,
        "#1152: max-width-object plus allow-output-object requests should fail during launch parsing");
    expect(visual_object_property(ambiguous_path, "one-guid", "MAXWIDTH") == "100",
        "#1152: max-width-object ambiguity should not mutate the asset");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void write_synthetic_form_table_for_object_max_width(const std::filesystem::path& form_path) {
    const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
        {.name = "OBJNAME", .type = 'C', .length = 24U},
        {.name = "NAME", .type = 'C', .length = 24U},
        {.name = "UNIQUEID", .type = 'C', .length = 32U},
        {.name = "MAXWIDTH", .type = 'N', .length = 10U, .decimal_count = 0U}
    };
    const std::vector<std::vector<std::string>> records{
        {"frmCustomer", "frmCustomer", "one-guid", "100"},
        {"frmOrder", "frmOrder", "two-guid", "200"},
        {"cntDetails", "cntDetails", "three-guid", "300"},
        {"frmOther", "frmOther", "other-guid", "400"}
    };

    const auto create_result = copperfin::vfp::create_dbf_table_file(form_path.string(), fields, records);
    expect(create_result.ok, "#1152: synthetic SCX table for object max width should be created");
}

void test_studio_host_json_assigns_max_left_by_stable_selectors(const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root = fs::temp_directory_path() / "copperfin_studio_host_max_left_object_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path max_left_path = temp_root / "max_left.scx";
    write_synthetic_form_table_for_object_max_left(max_left_path);
    const auto max_left_process = run_process_capture(
        studio_host_path,
        {
            "--path", max_left_path.string(),
            "--max-left-object",
            "--max-left", "640",
            "--max-left-target-object-name", "frmCustomer",
            "--max-left-target-unique-id", "two-guid",
            "--json"
        },
        temp_root);
    expect(max_left_process.exit_code == 0,
        "#1153: host object max-left assignment should exit successfully");
    expect(visual_object_property(max_left_path, "one-guid", "MAXLEFT") == "640" &&
            visual_object_property(max_left_path, "two-guid", "MAXLEFT") == "640" &&
            visual_object_property(max_left_path, "three-guid", "MAXLEFT") == "300" &&
            visual_object_property(max_left_path, "other-guid", "MAXLEFT") == "400",
        "#1153: host object max-left assignment should assign selected numeric value and preserve unrelated objects");

    const fs::path missing_target_path = temp_root / "missing_target.scx";
    write_synthetic_form_table_for_object_max_left(missing_target_path);
    const auto missing_target_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_target_path.string(),
            "--max-left-object",
            "--max-left", "640",
            "--max-left-target-unique-id", "one-guid",
            "--max-left-target-unique-id", "missing-guid",
            "--json"
        },
        temp_root);
    expect(missing_target_process.exit_code == 4,
        "#1153: missing-target host object max-left assignment should return command failure");
    expect(visual_object_property(missing_target_path, "one-guid", "MAXLEFT") == "100" &&
            visual_object_property(missing_target_path, "two-guid", "MAXLEFT") == "200",
        "#1153: missing-target host object max-left assignment should not mutate the asset");

    const fs::path missing_selector_path = temp_root / "missing_selector.scx";
    write_synthetic_form_table_for_object_max_left(missing_selector_path);
    const auto missing_selector_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_selector_path.string(),
            "--max-left-object",
            "--max-left", "640",
            "--json"
        },
        temp_root);
    expect(missing_selector_process.exit_code == 2,
        "#1153: max-left-object without target selectors should fail during launch parsing");
    expect(visual_object_property(missing_selector_path, "one-guid", "MAXLEFT") == "100",
        "#1153: max-left-object without target selectors should not mutate the asset");

    const fs::path missing_value_path = temp_root / "missing_value.scx";
    write_synthetic_form_table_for_object_max_left(missing_value_path);
    const auto missing_value_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_value_path.string(),
            "--max-left-object",
            "--max-left-target-unique-id", "one-guid",
            "--json"
        },
        temp_root);
    expect(missing_value_process.exit_code == 2,
        "#1153: max-left-object without max-left value should fail during launch parsing");
    expect(visual_object_property(missing_value_path, "one-guid", "MAXLEFT") == "100",
        "#1153: max-left-object without max-left value should not mutate the asset");

    const fs::path negative_value_path = temp_root / "negative_value.scx";
    write_synthetic_form_table_for_object_max_left(negative_value_path);
    const auto negative_value_process = run_process_capture(
        studio_host_path,
        {
            "--path", negative_value_path.string(),
            "--max-left-object",
            "--max-left", "-1",
            "--max-left-target-unique-id", "one-guid",
            "--json"
        },
        temp_root);
    expect(negative_value_process.exit_code == 2,
        "#1153: max-left-object with negative value should fail during launch parsing");
    expect(visual_object_property(negative_value_path, "one-guid", "MAXLEFT") == "100",
        "#1153: max-left-object with negative value should not mutate the asset");

    const fs::path duplicate_path = temp_root / "duplicate.scx";
    write_synthetic_form_table_for_object_max_left(duplicate_path);
    const auto duplicate_process = run_process_capture(
        studio_host_path,
        {
            "--path", duplicate_path.string(),
            "--max-left-object",
            "--max-left", "640",
            "--max-left-target-unique-id", "one-guid",
            "--max-left-target-object-name", "frmCustomer",
            "--json"
        },
        temp_root);
    expect(duplicate_process.exit_code == 4,
        "#1153: duplicate-target host object max-left assignment should return command failure");
    expect(visual_object_property(duplicate_path, "one-guid", "MAXLEFT") == "100",
        "#1153: duplicate-target host object max-left assignment should not mutate the asset");

    const fs::path ambiguous_path = temp_root / "ambiguous.scx";
    write_synthetic_form_table_for_object_max_left(ambiguous_path);
    const auto ambiguous_process = run_process_capture(
        studio_host_path,
        {
            "--path", ambiguous_path.string(),
            "--max-left-object",
            "--allow-output-object",
            "--max-left", "640",
            "--max-left-target-unique-id", "one-guid",
            "--allow-output", "false",
            "--allow-output-target-unique-id", "one-guid",
            "--json"
        },
        temp_root);
    expect(ambiguous_process.exit_code == 2,
        "#1153: max-left-object plus allow-output-object requests should fail during launch parsing");
    expect(visual_object_property(ambiguous_path, "one-guid", "MAXLEFT") == "100",
        "#1153: max-left-object ambiguity should not mutate the asset");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

}  // namespace cf_test_studio_host_json
