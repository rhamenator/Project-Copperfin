// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.

#include "test_studio_host_json_support.h"

namespace cf_test_studio_host_json {
void write_synthetic_form_table_for_object_caption(const std::filesystem::path& form_path) {
    const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
        {.name = "OBJNAME", .type = 'C', .length = 24U},
        {.name = "NAME", .type = 'C', .length = 24U},
        {.name = "UNIQUEID", .type = 'C', .length = 32U},
        {.name = "CAPTION", .type = 'C', .length = 48U}
    };
    const std::vector<std::vector<std::string>> records{
        {"cmdSave", "cmdSave", "one-guid", "Save"},
        {"cmdCancel", "cmdCancel", "two-guid", "Cancel"},
        {"lblStatus", "lblStatus", "three-guid", "Ready"},
        {"cmdOther", "cmdOther", "other-guid", "Other"}
    };

    const auto create_result = copperfin::vfp::create_dbf_table_file(form_path.string(), fields, records);
    expect(create_result.ok, "#1042: synthetic SCX table for object caption should be created");
}

void test_studio_host_json_assigns_caption_by_stable_selectors(const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root = fs::temp_directory_path() / "copperfin_studio_host_caption_object_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path caption_path = temp_root / "caption.scx";
    write_synthetic_form_table_for_object_caption(caption_path);
    const auto caption_process = run_process_capture(
        studio_host_path,
        {
            "--path", caption_path.string(),
            "--caption-object",
            "--caption", "Save Customer",
            "--caption-target-object-name", "cmdSave",
            "--caption-target-unique-id", "two-guid",
            "--json"
        },
        temp_root);
    expect(caption_process.exit_code == 0,
        "#1042: host object caption assignment should exit successfully");
    expect(visual_object_property(caption_path, "one-guid", "CAPTION") == "Save Customer" &&
            visual_object_property(caption_path, "two-guid", "CAPTION") == "Save Customer" &&
            visual_object_property(caption_path, "three-guid", "CAPTION") == "Ready" &&
            visual_object_property(caption_path, "other-guid", "CAPTION") == "Other",
        "#1042: host object caption assignment should assign selected text and preserve unrelated objects");

    const fs::path missing_target_path = temp_root / "missing_target.scx";
    write_synthetic_form_table_for_object_caption(missing_target_path);
    const auto missing_target_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_target_path.string(),
            "--caption-object",
            "--caption", "Save Customer",
            "--caption-target-unique-id", "one-guid",
            "--caption-target-unique-id", "missing-guid",
            "--json"
        },
        temp_root);
    expect(missing_target_process.exit_code == 4,
        "#1042: missing-target host object caption assignment should return command failure");
    expect(visual_object_property(missing_target_path, "one-guid", "CAPTION") == "Save" &&
            visual_object_property(missing_target_path, "two-guid", "CAPTION") == "Cancel",
        "#1042: missing-target host object caption assignment should not mutate the asset");

    const fs::path missing_selector_path = temp_root / "missing_selector.scx";
    write_synthetic_form_table_for_object_caption(missing_selector_path);
    const auto missing_selector_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_selector_path.string(),
            "--caption-object",
            "--caption", "Save Customer",
            "--json"
        },
        temp_root);
    expect(missing_selector_process.exit_code == 2,
        "#1042: caption-object without target selectors should fail during launch parsing");
    expect(visual_object_property(missing_selector_path, "one-guid", "CAPTION") == "Save",
        "#1042: caption-object without target selectors should not mutate the asset");

    const fs::path missing_value_path = temp_root / "missing_value.scx";
    write_synthetic_form_table_for_object_caption(missing_value_path);
    const auto missing_value_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_value_path.string(),
            "--caption-object",
            "--caption-target-unique-id", "one-guid",
            "--json"
        },
        temp_root);
    expect(missing_value_process.exit_code == 2,
        "#1042: caption-object without caption value should fail during launch parsing");
    expect(visual_object_property(missing_value_path, "one-guid", "CAPTION") == "Save",
        "#1042: caption-object without caption value should not mutate the asset");

    const fs::path duplicate_path = temp_root / "duplicate.scx";
    write_synthetic_form_table_for_object_caption(duplicate_path);
    const auto duplicate_process = run_process_capture(
        studio_host_path,
        {
            "--path", duplicate_path.string(),
            "--caption-object",
            "--caption", "Save Customer",
            "--caption-target-unique-id", "one-guid",
            "--caption-target-object-name", "cmdSave",
            "--json"
        },
        temp_root);
    expect(duplicate_process.exit_code == 4,
        "#1042: duplicate-target host object caption assignment should return command failure");
    expect(visual_object_property(duplicate_path, "one-guid", "CAPTION") == "Save",
        "#1042: duplicate-target host object caption assignment should not mutate the asset");

    const fs::path ambiguous_path = temp_root / "ambiguous.scx";
    write_synthetic_form_table_for_object_caption(ambiguous_path);
    const auto ambiguous_process = run_process_capture(
        studio_host_path,
        {
            "--path", ambiguous_path.string(),
            "--caption-object",
            "--locked-object",
            "--caption", "Save Customer",
            "--caption-target-unique-id", "one-guid",
            "--locked", "true",
            "--locked-target-unique-id", "one-guid",
            "--json"
        },
        temp_root);
    expect(ambiguous_process.exit_code == 2,
        "#1042: caption-object plus locked-object requests should fail during launch parsing");
    expect(visual_object_property(ambiguous_path, "one-guid", "CAPTION") == "Save",
        "#1042: caption-object ambiguity should not mutate the asset");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void write_synthetic_form_table_for_object_whats_this_help_id(const std::filesystem::path& form_path) {
    const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
        {.name = "OBJNAME", .type = 'C', .length = 24U},
        {.name = "NAME", .type = 'C', .length = 24U},
        {.name = "UNIQUEID", .type = 'C', .length = 32U},
        {.name = "WHATSTHISH", .type = 'N', .length = 6U, .decimal_count = 0U}
    };
    const std::vector<std::vector<std::string>> records{
        {"cmdSave", "cmdSave", "one-guid", "0"},
        {"cmdCancel", "cmdCancel", "two-guid", "1"},
        {"lblStatus", "lblStatus", "three-guid", "2"},
        {"cmdOther", "cmdOther", "other-guid", "0"}
    };

    const auto create_result = copperfin::vfp::create_dbf_table_file(form_path.string(), fields, records);
    expect(create_result.ok, "#1142: synthetic SCX table for object whats-this-help-id should be created");
}

void test_studio_host_json_assigns_whats_this_help_id_by_stable_selectors(const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root = fs::temp_directory_path() / "copperfin_studio_host_whats_this_help_id_object_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path whats_this_help_id_path = temp_root / "whats_this_help_id.scx";
    write_synthetic_form_table_for_object_whats_this_help_id(whats_this_help_id_path);
    const auto whats_this_help_id_process = run_process_capture(
        studio_host_path,
        {
            "--path", whats_this_help_id_path.string(),
            "--whats-this-help-id-object",
            "--whats-this-help-id", "900",
            "--whats-this-help-id-target-object-name", "cmdSave",
            "--whats-this-help-id-target-unique-id", "two-guid",
            "--json"
        },
        temp_root);
    expect(whats_this_help_id_process.exit_code == 0,
        "#1142: host object whats-this-help-id assignment should exit successfully");
    expect(visual_object_property(whats_this_help_id_path, "one-guid", "WHATSTHISHELPID") == "900" &&
            visual_object_property(whats_this_help_id_path, "two-guid", "WHATSTHISHELPID") == "900" &&
            visual_object_property(whats_this_help_id_path, "three-guid", "WHATSTHISHELPID") == "2" &&
            visual_object_property(whats_this_help_id_path, "other-guid", "WHATSTHISHELPID") == "0",
        "#1142: host object whats-this-help-id assignment should assign selected values and preserve unrelated objects");

    const fs::path missing_target_path = temp_root / "missing_target.scx";
    write_synthetic_form_table_for_object_whats_this_help_id(missing_target_path);
    const auto missing_target_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_target_path.string(),
            "--whats-this-help-id-object",
            "--whats-this-help-id", "2",
            "--whats-this-help-id-target-unique-id", "one-guid",
            "--whats-this-help-id-target-unique-id", "missing-guid",
            "--json"
        },
        temp_root);
    expect(missing_target_process.exit_code == 4,
        "#1142: missing-target host object whats-this-help-id assignment should return command failure");
    expect(visual_object_property(missing_target_path, "one-guid", "WHATSTHISHELPID") == "0" &&
            visual_object_property(missing_target_path, "two-guid", "WHATSTHISHELPID") == "1",
        "#1142: missing-target host object whats-this-help-id assignment should not mutate the asset");

    const fs::path missing_selector_path = temp_root / "missing_selector.scx";
    write_synthetic_form_table_for_object_whats_this_help_id(missing_selector_path);
    const auto missing_selector_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_selector_path.string(),
            "--whats-this-help-id-object",
            "--whats-this-help-id", "2",
            "--json"
        },
        temp_root);
    expect(missing_selector_process.exit_code == 2,
        "#1142: whats-this-help-id-object without target selectors should fail during launch parsing");
    expect(visual_object_property(missing_selector_path, "one-guid", "WHATSTHISHELPID") == "0",
        "#1142: whats-this-help-id-object without target selectors should not mutate the asset");

    const fs::path missing_value_path = temp_root / "missing_value.scx";
    write_synthetic_form_table_for_object_whats_this_help_id(missing_value_path);
    const auto missing_value_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_value_path.string(),
            "--whats-this-help-id-object",
            "--whats-this-help-id-target-unique-id", "one-guid",
            "--json"
        },
        temp_root);
    expect(missing_value_process.exit_code == 2,
        "#1142: whats-this-help-id-object without whats-this-help-id value should fail during launch parsing");
    expect(visual_object_property(missing_value_path, "one-guid", "WHATSTHISHELPID") == "0",
        "#1142: whats-this-help-id-object without whats-this-help-id value should not mutate the asset");

    const fs::path negative_value_path = temp_root / "negative_value.scx";
    write_synthetic_form_table_for_object_whats_this_help_id(negative_value_path);
    const auto negative_value_process = run_process_capture(
        studio_host_path,
        {
            "--path", negative_value_path.string(),
            "--whats-this-help-id-object",
            "--whats-this-help-id", "-1",
            "--whats-this-help-id-target-unique-id", "one-guid",
            "--json"
        },
        temp_root);
    expect(negative_value_process.exit_code == 2,
        "#1142: negative whats-this-help-id values should fail during launch parsing");
    expect(visual_object_property(negative_value_path, "one-guid", "WHATSTHISHELPID") == "0",
        "#1142: negative whats-this-help-id values should not mutate the asset");

    const fs::path duplicate_path = temp_root / "duplicate.scx";
    write_synthetic_form_table_for_object_whats_this_help_id(duplicate_path);
    const auto duplicate_process = run_process_capture(
        studio_host_path,
        {
            "--path", duplicate_path.string(),
            "--whats-this-help-id-object",
            "--whats-this-help-id", "2",
            "--whats-this-help-id-target-unique-id", "one-guid",
            "--whats-this-help-id-target-object-name", "cmdSave",
            "--json"
        },
        temp_root);
    expect(duplicate_process.exit_code == 4,
        "#1142: duplicate-target host object whats-this-help-id assignment should return command failure");
    expect(visual_object_property(duplicate_path, "one-guid", "WHATSTHISHELPID") == "0",
        "#1142: duplicate-target host object whats-this-help-id assignment should not mutate the asset");

    const fs::path ambiguous_path = temp_root / "ambiguous.scx";
    write_synthetic_form_table_for_object_whats_this_help_id(ambiguous_path);
    const auto ambiguous_process = run_process_capture(
        studio_host_path,
        {
            "--path", ambiguous_path.string(),
            "--whats-this-help-id-object",
            "--locked-object",
            "--whats-this-help-id", "2",
            "--whats-this-help-id-target-unique-id", "one-guid",
            "--locked", "true",
            "--locked-target-unique-id", "one-guid",
            "--json"
        },
        temp_root);
    expect(ambiguous_process.exit_code == 2,
        "#1142: whats-this-help-id-object plus locked-object requests should fail during launch parsing");
    expect(visual_object_property(ambiguous_path, "one-guid", "WHATSTHISHELPID") == "0",
        "#1142: whats-this-help-id-object ambiguity should not mutate the asset");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void write_synthetic_form_table_for_object_whats_this_help(const std::filesystem::path& form_path) {
    const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
        {.name = "OBJNAME", .type = 'C', .length = 24U},
        {.name = "NAME", .type = 'C', .length = 24U},
        {.name = "UNIQUEID", .type = 'C', .length = 32U},
        {.name = "WHATSTHISH", .type = 'C', .length = 10U}
    };
    const std::vector<std::vector<std::string>> records{
        {"cmdSave", "cmdSave", "one-guid", ".F."},
        {"cmdCancel", "cmdCancel", "two-guid", ".F."},
        {"lblStatus", "lblStatus", "three-guid", ".T."},
        {"cmdOther", "cmdOther", "other-guid", ".F."}
    };

    const auto create_result = copperfin::vfp::create_dbf_table_file(form_path.string(), fields, records);
    expect(create_result.ok, "#1143: synthetic SCX table for object whats-this-help should be created");
}

void test_studio_host_json_assigns_whats_this_help_by_stable_selectors(const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root = fs::temp_directory_path() / "copperfin_studio_host_whats_this_help_object_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path whats_this_help_path = temp_root / "whats_this_help.scx";
    write_synthetic_form_table_for_object_whats_this_help(whats_this_help_path);
    const auto whats_this_help_process = run_process_capture(
        studio_host_path,
        {
            "--path", whats_this_help_path.string(),
            "--whats-this-help-object",
            "--whats-this-help", "true",
            "--whats-this-help-target-object-name", "cmdSave",
            "--whats-this-help-target-unique-id", "two-guid",
            "--json"
        },
        temp_root);
    expect(whats_this_help_process.exit_code == 0,
        "#1143: host object whats-this-help assignment should exit successfully");
    expect(visual_object_property(whats_this_help_path, "one-guid", "WHATSTHISHELP") == ".T." &&
            visual_object_property(whats_this_help_path, "two-guid", "WHATSTHISHELP") == ".T." &&
            visual_object_property(whats_this_help_path, "three-guid", "WHATSTHISHELP") == ".T." &&
            visual_object_property(whats_this_help_path, "other-guid", "WHATSTHISHELP") == ".F.",
        "#1143: host object whats-this-help assignment should assign selected logical values and preserve unrelated objects");

    const fs::path missing_target_path = temp_root / "missing_target.scx";
    write_synthetic_form_table_for_object_whats_this_help(missing_target_path);
    const auto missing_target_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_target_path.string(),
            "--whats-this-help-object",
            "--whats-this-help", "true",
            "--whats-this-help-target-unique-id", "one-guid",
            "--whats-this-help-target-unique-id", "missing-guid",
            "--json"
        },
        temp_root);
    expect(missing_target_process.exit_code == 4,
        "#1143: missing-target host object whats-this-help assignment should return command failure");
    expect(visual_object_property(missing_target_path, "one-guid", "WHATSTHISHELP") == ".F." &&
            visual_object_property(missing_target_path, "two-guid", "WHATSTHISHELP") == ".F.",
        "#1143: missing-target host object whats-this-help assignment should not mutate the asset");

    const fs::path missing_selector_path = temp_root / "missing_selector.scx";
    write_synthetic_form_table_for_object_whats_this_help(missing_selector_path);
    const auto missing_selector_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_selector_path.string(),
            "--whats-this-help-object",
            "--whats-this-help", "true",
            "--json"
        },
        temp_root);
    expect(missing_selector_process.exit_code == 2,
        "#1143: whats-this-help-object without target selectors should fail during launch parsing");
    expect(visual_object_property(missing_selector_path, "one-guid", "WHATSTHISHELP") == ".F.",
        "#1143: whats-this-help-object without target selectors should not mutate the asset");

    const fs::path missing_value_path = temp_root / "missing_value.scx";
    write_synthetic_form_table_for_object_whats_this_help(missing_value_path);
    const auto missing_value_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_value_path.string(),
            "--whats-this-help-object",
            "--whats-this-help-target-unique-id", "one-guid",
            "--json"
        },
        temp_root);
    expect(missing_value_process.exit_code == 2,
        "#1143: whats-this-help-object without whats-this-help value should fail during launch parsing");
    expect(visual_object_property(missing_value_path, "one-guid", "WHATSTHISHELP") == ".F.",
        "#1143: whats-this-help-object without whats-this-help value should not mutate the asset");

    const fs::path duplicate_path = temp_root / "duplicate.scx";
    write_synthetic_form_table_for_object_whats_this_help(duplicate_path);
    const auto duplicate_process = run_process_capture(
        studio_host_path,
        {
            "--path", duplicate_path.string(),
            "--whats-this-help-object",
            "--whats-this-help", "true",
            "--whats-this-help-target-unique-id", "one-guid",
            "--whats-this-help-target-object-name", "cmdSave",
            "--json"
        },
        temp_root);
    expect(duplicate_process.exit_code == 4,
        "#1143: duplicate-target host object whats-this-help assignment should return command failure");
    expect(visual_object_property(duplicate_path, "one-guid", "WHATSTHISHELP") == ".F.",
        "#1143: duplicate-target host object whats-this-help assignment should not mutate the asset");

    const fs::path ambiguous_path = temp_root / "ambiguous.scx";
    write_synthetic_form_table_for_object_whats_this_help(ambiguous_path);
    const auto ambiguous_process = run_process_capture(
        studio_host_path,
        {
            "--path", ambiguous_path.string(),
            "--whats-this-help-object",
            "--locked-object",
            "--whats-this-help", "true",
            "--whats-this-help-target-unique-id", "one-guid",
            "--locked", "true",
            "--locked-target-unique-id", "one-guid",
            "--json"
        },
        temp_root);
    expect(ambiguous_process.exit_code == 2,
        "#1143: whats-this-help-object plus locked-object requests should fail during launch parsing");
    expect(visual_object_property(ambiguous_path, "one-guid", "WHATSTHISHELP") == ".F.",
        "#1143: whats-this-help-object ambiguity should not mutate the asset");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void write_synthetic_form_table_for_object_whats_this_button(const std::filesystem::path& form_path) {
    const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
        {.name = "OBJNAME", .type = 'C', .length = 24U},
        {.name = "NAME", .type = 'C', .length = 24U},
        {.name = "UNIQUEID", .type = 'C', .length = 32U},
        {.name = "WHATSTHISB", .type = 'C', .length = 10U}
    };
    const std::vector<std::vector<std::string>> records{
        {"cmdSave", "cmdSave", "one-guid", ".F."},
        {"cmdCancel", "cmdCancel", "two-guid", ".F."},
        {"lblStatus", "lblStatus", "three-guid", ".T."},
        {"cmdOther", "cmdOther", "other-guid", ".F."}
    };

    const auto create_result = copperfin::vfp::create_dbf_table_file(form_path.string(), fields, records);
    expect(create_result.ok, "#1144: synthetic SCX table for object whats-this-button should be created");
}

void test_studio_host_json_assigns_whats_this_button_by_stable_selectors(const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root = fs::temp_directory_path() / "copperfin_studio_host_whats_this_button_object_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path whats_this_button_path = temp_root / "whats_this_button.scx";
    write_synthetic_form_table_for_object_whats_this_button(whats_this_button_path);
    const auto whats_this_button_process = run_process_capture(
        studio_host_path,
        {
            "--path", whats_this_button_path.string(),
            "--whats-this-button-object",
            "--whats-this-button", "true",
            "--whats-this-button-target-object-name", "cmdSave",
            "--whats-this-button-target-unique-id", "two-guid",
            "--json"
        },
        temp_root);
    expect(whats_this_button_process.exit_code == 0,
        "#1144: host object whats-this-button assignment should exit successfully");
    expect(visual_object_property(whats_this_button_path, "one-guid", "WHATSTHISBUTTON") == ".T." &&
            visual_object_property(whats_this_button_path, "two-guid", "WHATSTHISBUTTON") == ".T." &&
            visual_object_property(whats_this_button_path, "three-guid", "WHATSTHISBUTTON") == ".T." &&
            visual_object_property(whats_this_button_path, "other-guid", "WHATSTHISBUTTON") == ".F.",
        "#1144: host object whats-this-button assignment should assign selected logical values and preserve unrelated objects");

    const fs::path missing_target_path = temp_root / "missing_target.scx";
    write_synthetic_form_table_for_object_whats_this_button(missing_target_path);
    const auto missing_target_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_target_path.string(),
            "--whats-this-button-object",
            "--whats-this-button", "true",
            "--whats-this-button-target-unique-id", "one-guid",
            "--whats-this-button-target-unique-id", "missing-guid",
            "--json"
        },
        temp_root);
    expect(missing_target_process.exit_code == 4,
        "#1144: missing-target host object whats-this-button assignment should return command failure");
    expect(visual_object_property(missing_target_path, "one-guid", "WHATSTHISBUTTON") == ".F." &&
            visual_object_property(missing_target_path, "two-guid", "WHATSTHISBUTTON") == ".F.",
        "#1144: missing-target host object whats-this-button assignment should not mutate the asset");

    const fs::path missing_selector_path = temp_root / "missing_selector.scx";
    write_synthetic_form_table_for_object_whats_this_button(missing_selector_path);
    const auto missing_selector_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_selector_path.string(),
            "--whats-this-button-object",
            "--whats-this-button", "true",
            "--json"
        },
        temp_root);
    expect(missing_selector_process.exit_code == 2,
        "#1144: whats-this-button-object without target selectors should fail during launch parsing");
    expect(visual_object_property(missing_selector_path, "one-guid", "WHATSTHISBUTTON") == ".F.",
        "#1144: whats-this-button-object without target selectors should not mutate the asset");

    const fs::path missing_value_path = temp_root / "missing_value.scx";
    write_synthetic_form_table_for_object_whats_this_button(missing_value_path);
    const auto missing_value_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_value_path.string(),
            "--whats-this-button-object",
            "--whats-this-button-target-unique-id", "one-guid",
            "--json"
        },
        temp_root);
    expect(missing_value_process.exit_code == 2,
        "#1144: whats-this-button-object without whats-this-button value should fail during launch parsing");
    expect(visual_object_property(missing_value_path, "one-guid", "WHATSTHISBUTTON") == ".F.",
        "#1144: whats-this-button-object without whats-this-button value should not mutate the asset");

    const fs::path duplicate_path = temp_root / "duplicate.scx";
    write_synthetic_form_table_for_object_whats_this_button(duplicate_path);
    const auto duplicate_process = run_process_capture(
        studio_host_path,
        {
            "--path", duplicate_path.string(),
            "--whats-this-button-object",
            "--whats-this-button", "true",
            "--whats-this-button-target-unique-id", "one-guid",
            "--whats-this-button-target-object-name", "cmdSave",
            "--json"
        },
        temp_root);
    expect(duplicate_process.exit_code == 4,
        "#1144: duplicate-target host object whats-this-button assignment should return command failure");
    expect(visual_object_property(duplicate_path, "one-guid", "WHATSTHISBUTTON") == ".F.",
        "#1144: duplicate-target host object whats-this-button assignment should not mutate the asset");

    const fs::path ambiguous_path = temp_root / "ambiguous.scx";
    write_synthetic_form_table_for_object_whats_this_button(ambiguous_path);
    const auto ambiguous_process = run_process_capture(
        studio_host_path,
        {
            "--path", ambiguous_path.string(),
            "--whats-this-button-object",
            "--locked-object",
            "--whats-this-button", "true",
            "--whats-this-button-target-unique-id", "one-guid",
            "--locked", "true",
            "--locked-target-unique-id", "one-guid",
            "--json"
        },
        temp_root);
    expect(ambiguous_process.exit_code == 2,
        "#1144: whats-this-button-object plus locked-object requests should fail during launch parsing");
    expect(visual_object_property(ambiguous_path, "one-guid", "WHATSTHISBUTTON") == ".F.",
        "#1144: whats-this-button-object ambiguity should not mutate the asset");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void write_synthetic_form_table_for_object_status_bar_text(const std::filesystem::path& form_path) {
    const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
        {.name = "OBJNAME", .type = 'C', .length = 24U},
        {.name = "NAME", .type = 'C', .length = 24U},
        {.name = "UNIQUEID", .type = 'C', .length = 32U},
        {.name = "STATUSBART", .type = 'C', .length = 70U}
    };
    const std::vector<std::vector<std::string>> records{
        {"cmdSave", "cmdSave", "one-guid", "Save"},
        {"cmdCancel", "cmdCancel", "two-guid", "Cancel"},
        {"lblStatus", "lblStatus", "three-guid", "Ready"},
        {"cmdOther", "cmdOther", "other-guid", "Other"}
    };

    const auto create_result = copperfin::vfp::create_dbf_table_file(form_path.string(), fields, records);
    expect(create_result.ok, "#1044: synthetic SCX table for object status-bar text should be created");
}

void test_studio_host_json_assigns_status_bar_text_by_stable_selectors(const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root = fs::temp_directory_path() / "copperfin_studio_host_status_bar_text_object_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path status_bar_text_path = temp_root / "status_bar_text.scx";
    write_synthetic_form_table_for_object_status_bar_text(status_bar_text_path);
    const auto status_bar_text_process = run_process_capture(
        studio_host_path,
        {
            "--path", status_bar_text_path.string(),
            "--status-bar-text-object",
            "--status-bar-text", "Ready to save",
            "--status-bar-text-target-object-name", "cmdSave",
            "--status-bar-text-target-unique-id", "two-guid",
            "--json"
        },
        temp_root);
    expect(status_bar_text_process.exit_code == 0,
        "#1044: host object status-bar text assignment should exit successfully");
    expect(visual_object_property(status_bar_text_path, "one-guid", "STATUSBARTEXT") == "Ready to save" &&
            visual_object_property(status_bar_text_path, "two-guid", "STATUSBARTEXT") == "Ready to save" &&
            visual_object_property(status_bar_text_path, "three-guid", "STATUSBARTEXT") == "Ready" &&
            visual_object_property(status_bar_text_path, "other-guid", "STATUSBARTEXT") == "Other",
        "#1044: host object status-bar text assignment should assign selected text and preserve unrelated objects");

    const fs::path missing_target_path = temp_root / "missing_target.scx";
    write_synthetic_form_table_for_object_status_bar_text(missing_target_path);
    const auto missing_target_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_target_path.string(),
            "--status-bar-text-object",
            "--status-bar-text", "Ready to save",
            "--status-bar-text-target-unique-id", "one-guid",
            "--status-bar-text-target-unique-id", "missing-guid",
            "--json"
        },
        temp_root);
    expect(missing_target_process.exit_code == 4,
        "#1044: missing-target host object status-bar text assignment should return command failure");
    expect(visual_object_property(missing_target_path, "one-guid", "STATUSBARTEXT") == "Save" &&
            visual_object_property(missing_target_path, "two-guid", "STATUSBARTEXT") == "Cancel",
        "#1044: missing-target host object status-bar text assignment should not mutate the asset");

    const fs::path missing_selector_path = temp_root / "missing_selector.scx";
    write_synthetic_form_table_for_object_status_bar_text(missing_selector_path);
    const auto missing_selector_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_selector_path.string(),
            "--status-bar-text-object",
            "--status-bar-text", "Ready to save",
            "--json"
        },
        temp_root);
    expect(missing_selector_process.exit_code == 2,
        "#1044: status-bar-text-object without target selectors should fail during launch parsing");
    expect(visual_object_property(missing_selector_path, "one-guid", "STATUSBARTEXT") == "Save",
        "#1044: status-bar-text-object without target selectors should not mutate the asset");

    const fs::path missing_value_path = temp_root / "missing_value.scx";
    write_synthetic_form_table_for_object_status_bar_text(missing_value_path);
    const auto missing_value_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_value_path.string(),
            "--status-bar-text-object",
            "--status-bar-text-target-unique-id", "one-guid",
            "--json"
        },
        temp_root);
    expect(missing_value_process.exit_code == 2,
        "#1044: status-bar-text-object without status-bar text value should fail during launch parsing");
    expect(visual_object_property(missing_value_path, "one-guid", "STATUSBARTEXT") == "Save",
        "#1044: status-bar-text-object without status-bar text value should not mutate the asset");

    const fs::path duplicate_path = temp_root / "duplicate.scx";
    write_synthetic_form_table_for_object_status_bar_text(duplicate_path);
    const auto duplicate_process = run_process_capture(
        studio_host_path,
        {
            "--path", duplicate_path.string(),
            "--status-bar-text-object",
            "--status-bar-text", "Ready to save",
            "--status-bar-text-target-unique-id", "one-guid",
            "--status-bar-text-target-object-name", "cmdSave",
            "--json"
        },
        temp_root);
    expect(duplicate_process.exit_code == 4,
        "#1044: duplicate-target host object status-bar text assignment should return command failure");
    expect(visual_object_property(duplicate_path, "one-guid", "STATUSBARTEXT") == "Save",
        "#1044: duplicate-target host object status-bar text assignment should not mutate the asset");

    const fs::path ambiguous_path = temp_root / "ambiguous.scx";
    write_synthetic_form_table_for_object_status_bar_text(ambiguous_path);
    const auto ambiguous_process = run_process_capture(
        studio_host_path,
        {
            "--path", ambiguous_path.string(),
            "--status-bar-text-object",
            "--tooltip-text-object",
            "--status-bar-text", "Ready to save",
            "--status-bar-text-target-unique-id", "one-guid",
            "--tooltip-text", "Save this customer",
            "--tooltip-text-target-unique-id", "one-guid",
            "--json"
        },
        temp_root);
    expect(ambiguous_process.exit_code == 2,
        "#1044: status-bar-text-object plus tooltip-text-object requests should fail during launch parsing");
    expect(visual_object_property(ambiguous_path, "one-guid", "STATUSBARTEXT") == "Save",
        "#1044: status-bar-text-object ambiguity should not mutate the asset");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

}  // namespace cf_test_studio_host_json
