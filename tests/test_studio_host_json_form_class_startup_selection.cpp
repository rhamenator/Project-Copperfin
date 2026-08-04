// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only

#include "test_studio_host_json_support.h"

namespace cf_test_studio_host_json {
namespace {

void write_synthetic_form_or_class_table(const std::filesystem::path& path) {
    const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
        {.name = "OBJNAME", .type = 'C', .length = 32U},
        {.name = "UNIQUEID", .type = 'C', .length = 40U},
        {.name = "PARENT", .type = 'C', .length = 32U},
        {.name = "CLASS", .type = 'C', .length = 32U},
        {.name = "BASECLASS", .type = 'C', .length = 32U}
    };
    std::vector<std::vector<std::string>> records{
        {"FormRoot", "form-root-guid", "", "Form", "Form"}
    };
    for (int index = 1; index <= 8; ++index) {
        records.push_back({
            "Filler" + std::to_string(index),
            "filler-" + std::to_string(index) + "-guid",
            "FormRoot",
            "CommandButton",
            "CommandButton"
        });
    }
    records.push_back({
        "TargetObject",
        "target-object-guid",
        "FormRoot",
        "CommandButton",
        "CommandButton"
    });

    const auto create_result = copperfin::vfp::create_dbf_table_file(path.string(), fields, records);
    expect(create_result.ok,
           "#4280: synthetic form/class table should be created with a target beyond the default preview");
}

void assert_selected_startup_object(
    const ProcessResult& process,
    const std::string& asset_kind,
    const std::string& selector_description) {
    expect(process.exit_code == 0,
           "#4280: " + selector_description + " startup selection should exit successfully");
    expect_contains(process.stdout_text, "\"kind\": \"" + asset_kind + "\"",
                    "#4280: " + selector_description + " should preserve the asset kind");
    expect_contains(process.stdout_text, "\"recordAvailable\": true",
                    "#4280: " + selector_description + " should advertise a selected record");
    expect_contains(process.stdout_text, "\"recordIndex\": 9",
                    "#4280: " + selector_description + " should resolve the object beyond the default preview");
    expect_contains(process.stdout_text, "\"selectedObjectAvailable\": true",
                    "#4280: " + selector_description + " should expose the selected object");
    expect_contains(process.stdout_text, "\"objectName\": \"TargetObject\"",
                    "#4280: " + selector_description + " should expose the selected object name");
    expect_contains(process.stdout_text, "\"uniqueId\": \"target-object-guid\"",
                    "#4280: " + selector_description + " should expose the selected object identity");
}

}  // namespace

void test_studio_host_json_selects_form_and_class_objects_on_open(const std::string& studio_host_path) {
    namespace fs = std::filesystem;
    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_form_class_startup_selection_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);
    ScopedDefaultLocaleCatalogEnvironment default_locale_environment;

    const fs::path form_path = temp_root / "startup.scx";
    write_synthetic_form_or_class_table(form_path);
    const auto form_process = run_process_capture(
        studio_host_path,
        {"--path", form_path.string(), "--object-name", "TargetObject", "--json"},
        temp_root);
    assert_selected_startup_object(form_process, "form", "form object-name");

    const fs::path class_path = temp_root / "startup.vcx";
    write_synthetic_form_or_class_table(class_path);
    const auto class_process = run_process_capture(
        studio_host_path,
        {"--path", class_path.string(), "--unique-id", "target-object-guid", "--json"},
        temp_root);
    assert_selected_startup_object(class_process, "class_library", "class unique-id");

    fs::remove_all(temp_root, ignored);
}

}  // namespace cf_test_studio_host_json
