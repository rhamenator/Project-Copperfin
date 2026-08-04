// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.

#include "test_studio_host_json_support.h"

namespace cf_test_studio_host_json {
void test_studio_host_json_preserves_sidecar_path_spelling(const std::string& studio_host_path) {
    namespace fs = std::filesystem;
    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_json_sidecar_spelling";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);
    ScopedDefaultLocaleCatalogEnvironment default_locale_environment;

    const fs::path form_path = temp_root / "sample.scx";
    const fs::path inferred_sidecar_path = temp_root / "sample.sct";
    const fs::path temporary_sidecar_path = temp_root / "sidecar.rename";
    const fs::path actual_sidecar_path = temp_root / "Sample.SCT";
    const auto create_result = copperfin::vfp::create_dbf_table_file(
        form_path.string(),
        {{.name = "OBJNAME", .type = 'M', .length = 4U}},
        {{"sample-object"}});
    expect(create_result.ok, "#3992: mixed-case sidecar JSON fixture should be created");
    fs::rename(inferred_sidecar_path, temporary_sidecar_path, ignored);
    ignored.clear();
    fs::rename(temporary_sidecar_path, actual_sidecar_path, ignored);

    const auto process = run_process_capture(
        studio_host_path,
        {"--path", form_path.string(), "--json"},
        temp_root);
    if (process.exit_code != 0) {
        std::cerr << "studio host sidecar spelling stdout:\n" << process.stdout_text << "\n";
        std::cerr << "studio host sidecar spelling stderr:\n" << process.stderr_text << "\n";
        std::cerr << "fixture root: " << temp_root << "\n";
    }

    expect(process.exit_code == 0,
           "#3992: mixed-case sidecar JSON smoke should exit successfully");
    const std::size_t sidecar_key = process.stdout_text.find("\"sidecarPath\":");
    const std::size_t sidecar_line_end = sidecar_key == std::string::npos
        ? std::string::npos
        : process.stdout_text.find('\n', sidecar_key);
    const std::string sidecar_line = sidecar_key == std::string::npos
        ? std::string{}
        : process.stdout_text.substr(sidecar_key, sidecar_line_end - sidecar_key);
    expect(sidecar_line.find(actual_sidecar_path.filename().string()) != std::string::npos &&
               sidecar_line.find(inferred_sidecar_path.filename().string()) == std::string::npos,
           "#3992: sidecarPath JSON should preserve actual directory-entry spelling");
    expect_contains(process.stdout_text, "\"hasSidecar\": true",
                    "#3992: mixed-case sidecar JSON should report companion availability");

    fs::remove_all(temp_root, ignored);
}

void test_studio_host_json_exposes_designer_contexts(const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root = fs::temp_directory_path() / "copperfin_studio_host_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);
    ScopedDefaultLocaleCatalogEnvironment default_locale_environment;

    const fs::path form_path = temp_root / "customer.scx";
    write_synthetic_form_asset(form_path);

    const auto process = run_process_capture(
        studio_host_path,
        {"--path", form_path.string(), "--json"},
        temp_root);

    if (process.exit_code != 0) {
        std::cerr << "studio host stdout:\n" << process.stdout_text << "\n";
        std::cerr << "studio host stderr:\n" << process.stderr_text << "\n";
        std::cerr << "fixture root: " << temp_root << "\n";
    }

    expect(process.exit_code == 0, "#961: Studio host JSON smoke should exit successfully");
    expect_contains(process.stdout_text, "\"designerContexts\": [",
                    "#961: document JSON should expose designer context array");
    expect_contains(process.stdout_text, "\"selectedReportSelectionAvailable\": false",
                    "#1457: non-report JSON should not advertise report-selection availability");
    expect_contains(process.stdout_text, "\"selectedReportSelectionKind\": \"none\"",
                    "#1457: non-report JSON should expose the none report-selection kind");
    expect_contains(process.stdout_text, "\"selectionContext\": \"visual_object\"",
                    "#961: form JSON should expose the visual-object context token");
    expect_contains(process.stdout_text, "\"editorActionCount\": 5",
                    "#1009: form JSON should expose designer context editor-action count");
    expect_contains(process.stdout_text, "\"builderCount\": 3",
                    "#1009: form JSON should expose designer context builder count");
    expect_contains(process.stdout_text, "\"toolboxItemCount\": ",
                    "#1009: form JSON should expose designer context toolbox-item count");
    expect_contains(process.stdout_text, "\"editorActions\": [",
                    "#961: designer context JSON should expose editor actions");
    expect_contains(process.stdout_text, "\"id\": \"show-property-grid\"",
                    "#961: designer context JSON should expose property-grid action ids");
    expect_contains(process.stdout_text, "\"builders\": [",
                    "#961: designer context JSON should expose builders");
    expect_contains(process.stdout_text, "\"id\": \"form-builder\"",
                    "#1010: designer context JSON should expose form builder ids");
    expect_contains(process.stdout_text, "\"id\": \"control-builder\"",
                    "#961: designer context JSON should expose control builder ids");
    expect_contains(process.stdout_text, "\"toolboxItems\": [",
                    "#961: designer context JSON should expose toolbox items");
    expect_contains(process.stdout_text, "\"id\": \"textbox\"",
                    "#961: designer context JSON should expose TextBox toolbox ids");

    const auto override_process = run_process_capture(
        studio_host_path,
        {"--path", form_path.string(), "--selection-context", "visual_method", "--json"},
        temp_root);

    if (override_process.exit_code != 0) {
        std::cerr << "studio host override stdout:\n" << override_process.stdout_text << "\n";
        std::cerr << "studio host override stderr:\n" << override_process.stderr_text << "\n";
        std::cerr << "fixture root: " << temp_root << "\n";
    }

    expect(override_process.exit_code == 0, "#962: Studio host explicit context JSON smoke should exit successfully");
    expect_contains(override_process.stdout_text, "\"selectionContext\": \"visual_method\"",
                    "#962: explicit visual_method selection contexts should serialize through host JSON");
    expect_contains(override_process.stdout_text, "\"id\": \"edit-visual-method\"",
                    "#962: explicit visual_method contexts should expose method-editor actions");
    expect_not_contains(override_process.stdout_text, "\"selectionContext\": \"visual_object\"",
                        "#962: explicit selection contexts should override the form default selection context");

    const auto container_override_process = run_process_capture(
        studio_host_path,
        {"--path", form_path.string(), "--selection-context", "container_object", "--json"},
        temp_root);

    if (container_override_process.exit_code != 0) {
        std::cerr << "studio host container override stdout:\n" << container_override_process.stdout_text << "\n";
        std::cerr << "studio host container override stderr:\n" << container_override_process.stderr_text << "\n";
        std::cerr << "fixture root: " << temp_root << "\n";
    }

    expect(container_override_process.exit_code == 0,
           "#1014: Studio host explicit container context JSON smoke should exit successfully");
    expect_contains(container_override_process.stdout_text, "\"selectionContext\": \"container_object\"",
                    "#1014: explicit container_object selection contexts should serialize through host JSON");
    expect_contains(container_override_process.stdout_text, "\"id\": \"control-builder\"",
                    "#1014: explicit container_object contexts should expose control builder metadata");
    expect_contains(container_override_process.stdout_text, "\"id\": \"grid-builder\"",
                    "#1014: explicit container_object contexts should expose grid builder metadata");
    expect_contains(container_override_process.stdout_text, "\"id\": \"checkbox\"",
                    "#1014: explicit container_object contexts should expose container-safe toolbox metadata");
    expect_not_contains(container_override_process.stdout_text, "\"id\": \"form-builder\"",
                        "#1014: explicit container_object contexts should not expose form builders");
    expect_not_contains(container_override_process.stdout_text, "\"id\": \"class-builder\"",
                        "#1014: explicit container_object contexts should not expose class builders");

    const auto report_override_process = run_process_capture(
        studio_host_path,
        {"--path", form_path.string(), "--selection-context", "report_expression", "--json"},
        temp_root);

    if (report_override_process.exit_code != 0) {
        std::cerr << "studio host report override stdout:\n" << report_override_process.stdout_text << "\n";
        std::cerr << "studio host report override stderr:\n" << report_override_process.stderr_text << "\n";
        std::cerr << "fixture root: " << temp_root << "\n";
    }

    expect(report_override_process.exit_code == 0,
           "#2072: Studio host explicit report context JSON smoke should exit successfully");
    expect_contains(report_override_process.stdout_text, "\"selectionContext\": \"report_expression\"",
                    "#2072: explicit report_expression selection contexts should serialize through host JSON");
    expect_contains(report_override_process.stdout_text, "\"editorActionCount\": 5",
                    "#2078: explicit report_expression contexts should expose the full report/label editor-action set");
    expect_contains(report_override_process.stdout_text, "\"id\": \"edit-report-expression\"",
                    "#2078: explicit report_expression contexts should expose expression editor actions");
    expect_contains(report_override_process.stdout_text, "\"id\": \"show-toolbox\"",
                    "#2078: explicit report_expression contexts should expose toolbox editor actions");
    expect_contains(report_override_process.stdout_text, "\"id\": \"edit-data-environment\"",
                    "#2078: explicit report_expression contexts should expose data-environment editor actions");
    expect_contains(report_override_process.stdout_text, "\"toolboxItemCount\": 4",
                    "#2078: explicit report_expression contexts should expose report-safe toolbox item counts");
    expect_contains(report_override_process.stdout_text, "\"id\": \"label\"",
                    "#2079: explicit report_expression contexts should expose report label toolbox items");
    expect_contains(report_override_process.stdout_text, "\"id\": \"image\"",
                    "#2079: explicit report_expression contexts should expose report image toolbox items");
    expect_contains(report_override_process.stdout_text, "\"id\": \"line\"",
                    "#2079: explicit report_expression contexts should expose report line toolbox items");
    expect_contains(report_override_process.stdout_text, "\"id\": \"shape\"",
                    "#2079: explicit report_expression contexts should expose report shape toolbox items");
    expect_not_contains(report_override_process.stdout_text, "\"id\": \"textbox\"",
                        "#2078: explicit report_expression contexts should not expose form-only TextBox toolbox items");
    expect_contains(report_override_process.stdout_text, "\"id\": \"report-builder\"",
                    "#2072: explicit report_expression contexts should expose report builder metadata");
    expect_not_contains(report_override_process.stdout_text, "\"id\": \"label-wizard\"",
                        "#2072: explicit report_expression contexts should not reuse label wizard metadata");

    const auto label_override_process = run_process_capture(
        studio_host_path,
        {"--path", form_path.string(), "--selection-context", "label_expression", "--json"},
        temp_root);

    if (label_override_process.exit_code != 0) {
        std::cerr << "studio host label override stdout:\n" << label_override_process.stdout_text << "\n";
        std::cerr << "studio host label override stderr:\n" << label_override_process.stderr_text << "\n";
        std::cerr << "fixture root: " << temp_root << "\n";
    }

    expect(label_override_process.exit_code == 0,
           "#1011: Studio host explicit label context JSON smoke should exit successfully");
    expect_contains(label_override_process.stdout_text, "\"selectionContext\": \"label_expression\"",
                    "#1011: explicit label_expression selection contexts should serialize through host JSON");
    expect_contains(label_override_process.stdout_text, "\"editorActionCount\": 5",
                    "#2078: explicit label_expression contexts should expose the full report/label editor-action set");
    expect_contains(label_override_process.stdout_text, "\"id\": \"edit-report-expression\"",
                    "#2078: explicit label_expression contexts should expose expression editor actions");
    expect_contains(label_override_process.stdout_text, "\"id\": \"show-toolbox\"",
                    "#2078: explicit label_expression contexts should expose toolbox editor actions");
    expect_contains(label_override_process.stdout_text, "\"id\": \"edit-data-environment\"",
                    "#2078: explicit label_expression contexts should expose data-environment editor actions");
    expect_contains(label_override_process.stdout_text, "\"toolboxItemCount\": 4",
                    "#2078: explicit label_expression contexts should expose report-safe toolbox item counts");
    expect_contains(label_override_process.stdout_text, "\"id\": \"label\"",
                    "#2079: explicit label_expression contexts should expose report label toolbox items");
    expect_contains(label_override_process.stdout_text, "\"id\": \"image\"",
                    "#2079: explicit label_expression contexts should expose report image toolbox items");
    expect_contains(label_override_process.stdout_text, "\"id\": \"line\"",
                    "#2079: explicit label_expression contexts should expose report line toolbox items");
    expect_contains(label_override_process.stdout_text, "\"id\": \"shape\"",
                    "#2079: explicit label_expression contexts should expose report shape toolbox items");
    expect_not_contains(label_override_process.stdout_text, "\"id\": \"textbox\"",
                        "#2078: explicit label_expression contexts should not expose form-only TextBox toolbox items");
    expect_contains(label_override_process.stdout_text, "\"id\": \"label-wizard\"",
                    "#1011: explicit label_expression contexts should expose label wizard builders");
    expect_not_contains(label_override_process.stdout_text, "\"id\": \"report-builder\"",
                        "#1011: explicit label_expression contexts should not reuse report builders");

    const auto class_override_process = run_process_capture(
        studio_host_path,
        {"--path", form_path.string(), "--selection-context", "class_designer", "--json"},
        temp_root);

    if (class_override_process.exit_code != 0) {
        std::cerr << "studio host class override stdout:\n" << class_override_process.stdout_text << "\n";
        std::cerr << "studio host class override stderr:\n" << class_override_process.stderr_text << "\n";
        std::cerr << "fixture root: " << temp_root << "\n";
    }

    expect(class_override_process.exit_code == 0,
           "#1012: Studio host explicit class context JSON smoke should exit successfully");
    expect_contains(class_override_process.stdout_text, "\"selectionContext\": \"class_designer\"",
                    "#1012: explicit class_designer selection contexts should serialize through host JSON");
    expect_contains(class_override_process.stdout_text, "\"id\": \"class-builder\"",
                    "#1012: explicit class_designer contexts should expose class builder metadata");
    expect_not_contains(class_override_process.stdout_text, "\"id\": \"form-builder\"",
                        "#1012: explicit class_designer contexts should not expose form builders");
    expect_not_contains(class_override_process.stdout_text, "\"id\": \"control-builder\"",
                        "#1012: explicit class_designer contexts should not expose control builders");

    const auto menu_override_process = run_process_capture(
        studio_host_path,
        {"--path", form_path.string(), "--selection-context", "menu_item", "--json"},
        temp_root);

    if (menu_override_process.exit_code != 0) {
        std::cerr << "studio host menu override stdout:\n" << menu_override_process.stdout_text << "\n";
        std::cerr << "studio host menu override stderr:\n" << menu_override_process.stderr_text << "\n";
        std::cerr << "fixture root: " << temp_root << "\n";
    }

    expect(menu_override_process.exit_code == 0,
           "#1013: Studio host explicit menu context JSON smoke should exit successfully");
    expect_contains(menu_override_process.stdout_text, "\"selectionContext\": \"menu_item\"",
                    "#1013: explicit menu_item selection contexts should serialize through host JSON");
    expect_contains(menu_override_process.stdout_text, "\"id\": \"menu-designer\"",
                    "#1013: explicit menu_item contexts should expose menu designer metadata");
    expect_contains(menu_override_process.stdout_text, "\"toolboxItemCount\": 0",
                    "#1013: explicit menu_item contexts should expose zero toolbox-item count");
    expect_not_contains(menu_override_process.stdout_text, "\"id\": \"form-builder\"",
                        "#1013: explicit menu_item contexts should not expose form builders");

    const fs::path label_path = temp_root / "mailing.lbx";
    write_synthetic_form_asset(label_path);
    const auto label_process = run_process_capture(
        studio_host_path,
        {"--path", label_path.string(), "--json"},
        temp_root);

    if (label_process.exit_code != 0) {
        std::cerr << "studio host label stdout:\n" << label_process.stdout_text << "\n";
        std::cerr << "studio host label stderr:\n" << label_process.stderr_text << "\n";
        std::cerr << "fixture root: " << temp_root << "\n";
    }

    expect(label_process.exit_code == 0, "#1011: Studio host label JSON smoke should exit successfully");
    expect_contains(label_process.stdout_text, "\"kind\": \"label\"",
                    "#1011: label JSON should preserve label document kind");
    expect_contains(label_process.stdout_text, "\"selectionContext\": \"label_expression\"",
                    "#1011: label documents should default to label-expression JSON contexts");
    expect_contains(label_process.stdout_text, "\"editorActionCount\": 5",
                    "#2076: label expression contexts should expose the full report/label editor-action set");
    expect_contains(label_process.stdout_text, "\"id\": \"edit-report-expression\"",
                    "#2076: label expression contexts should expose expression editor actions");
    expect_contains(label_process.stdout_text, "\"id\": \"show-toolbox\"",
                    "#2076: label expression contexts should expose toolbox editor actions");
    expect_contains(label_process.stdout_text, "\"id\": \"edit-data-environment\"",
                    "#2076: label expression contexts should expose data-environment editor actions");
    expect_contains(label_process.stdout_text, "\"toolboxItemCount\": 4",
                    "#2077: label expression contexts should expose report-safe toolbox item counts");
    expect_contains(label_process.stdout_text, "\"id\": \"label\"",
                    "#2077: label expression contexts should expose report label toolbox items");
    expect_contains(label_process.stdout_text, "\"id\": \"image\"",
                    "#2077: label expression contexts should expose report image toolbox items");
    expect_contains(label_process.stdout_text, "\"id\": \"line\"",
                    "#2077: label expression contexts should expose report line toolbox items");
    expect_contains(label_process.stdout_text, "\"id\": \"shape\"",
                    "#2077: label expression contexts should expose report shape toolbox items");
    expect_not_contains(label_process.stdout_text, "\"id\": \"textbox\"",
                        "#2077: label expression contexts should not expose form-only TextBox toolbox items");
    expect_contains(label_process.stdout_text, "\"builderCount\": 1",
                    "#1011: label JSON should expose label builder count");
    expect_contains(label_process.stdout_text, "\"id\": \"label-wizard\"",
                    "#1011: label JSON should expose label wizard builder ids");
    expect_not_contains(label_process.stdout_text, "\"id\": \"report-builder\"",
                        "#1011: label JSON should not expose report builder ids");

    const fs::path report_path = temp_root / "summary.frx";
    write_synthetic_form_asset(report_path);
    const auto report_process = run_process_capture(
        studio_host_path,
        {"--path", report_path.string(), "--json"},
        temp_root);

    if (report_process.exit_code != 0) {
        std::cerr << "studio host report stdout:\n" << report_process.stdout_text << "\n";
        std::cerr << "studio host report stderr:\n" << report_process.stderr_text << "\n";
        std::cerr << "fixture root: " << temp_root << "\n";
    }

    expect(report_process.exit_code == 0,
           "#2073: Studio host report JSON smoke should exit successfully");
    expect_contains(report_process.stdout_text, "\"kind\": \"report\"",
                    "#2073: report JSON should preserve report document kind");
    expect_contains(report_process.stdout_text, "\"selectionContext\": \"report_expression\"",
                    "#2073: report documents should default to report-expression JSON contexts");
    expect_contains(report_process.stdout_text, "\"editorActionCount\": 5",
                    "#2076: report expression contexts should expose the full report/label editor-action set");
    expect_contains(report_process.stdout_text, "\"id\": \"edit-report-expression\"",
                    "#2076: report expression contexts should expose expression editor actions");
    expect_contains(report_process.stdout_text, "\"id\": \"show-toolbox\"",
                    "#2076: report expression contexts should expose toolbox editor actions");
    expect_contains(report_process.stdout_text, "\"id\": \"edit-data-environment\"",
                    "#2076: report expression contexts should expose data-environment editor actions");
    expect_contains(report_process.stdout_text, "\"toolboxItemCount\": 4",
                    "#2077: report expression contexts should expose report-safe toolbox item counts");
    expect_contains(report_process.stdout_text, "\"id\": \"label\"",
                    "#2077: report expression contexts should expose report label toolbox items");
    expect_contains(report_process.stdout_text, "\"id\": \"image\"",
                    "#2077: report expression contexts should expose report image toolbox items");
    expect_contains(report_process.stdout_text, "\"id\": \"line\"",
                    "#2077: report expression contexts should expose report line toolbox items");
    expect_contains(report_process.stdout_text, "\"id\": \"shape\"",
                    "#2077: report expression contexts should expose report shape toolbox items");
    expect_not_contains(report_process.stdout_text, "\"id\": \"textbox\"",
                        "#2077: report expression contexts should not expose form-only TextBox toolbox items");
    expect_contains(report_process.stdout_text, "\"builderCount\": 1",
                    "#2073: report JSON should expose report builder count");
    expect_contains(report_process.stdout_text, "\"id\": \"report-builder\"",
                    "#2073: report JSON should expose report builder ids");
    expect_not_contains(report_process.stdout_text, "\"id\": \"label-wizard\"",
                        "#2073: report JSON should not expose label wizard ids");

    const auto report_data_environment_symbol_process = run_process_capture(
        studio_host_path,
        {"--path", report_path.string(), "--symbol", "Dataenvironment.OpenTables", "--json"},
        temp_root);

    if (report_data_environment_symbol_process.exit_code != 0) {
        std::cerr << "studio host report data-environment symbol stdout:\n"
                  << report_data_environment_symbol_process.stdout_text << "\n";
        std::cerr << "studio host report data-environment symbol stderr:\n"
                  << report_data_environment_symbol_process.stderr_text << "\n";
        std::cerr << "fixture root: " << temp_root << "\n";
    }

    expect(report_data_environment_symbol_process.exit_code == 0,
           "#1016: Studio host report DataEnvironment symbol JSON smoke should exit successfully");
    expect_contains(report_data_environment_symbol_process.stdout_text, "\"selectionContext\": \"data_environment\"",
                    "#1016: report DataEnvironment symbols should infer data-environment JSON contexts");
    expect_contains(report_data_environment_symbol_process.stdout_text, "\"toolboxItemCount\": 0",
                    "#2075: report DataEnvironment symbols should expose zero toolbox-item count");
    expect_contains(report_data_environment_symbol_process.stdout_text, "\"id\": \"edit-data-environment\"",
                    "#2075: report DataEnvironment symbols should expose data-environment editor actions");
    expect_contains(report_data_environment_symbol_process.stdout_text, "\"id\": \"data-environment-builder\"",
                    "#1016: report DataEnvironment symbols should expose data-environment builders");
    expect_not_contains(report_data_environment_symbol_process.stdout_text, "\"selectionContext\": \"report_expression\"",
                        "#2074: report DataEnvironment symbols should not keep report-expression contexts");
    expect_not_contains(report_data_environment_symbol_process.stdout_text, "\"selectionContext\": \"label_expression\"",
                        "#1016: DataEnvironment symbols should override report/label expression defaults");
    expect_not_contains(report_data_environment_symbol_process.stdout_text, "\"id\": \"report-builder\"",
                        "#2074: report DataEnvironment symbols should not expose report builder metadata");
    expect_not_contains(report_data_environment_symbol_process.stdout_text, "\"id\": \"label-wizard\"",
                        "#2074: report DataEnvironment symbols should not expose label wizard metadata");

    const fs::path class_path = temp_root / "customer.vcx";
    write_synthetic_form_asset(class_path);
    const auto class_process = run_process_capture(
        studio_host_path,
        {"--path", class_path.string(), "--json"},
        temp_root);

    if (class_process.exit_code != 0) {
        std::cerr << "studio host class stdout:\n" << class_process.stdout_text << "\n";
        std::cerr << "studio host class stderr:\n" << class_process.stderr_text << "\n";
        std::cerr << "fixture root: " << temp_root << "\n";
    }

    expect(class_process.exit_code == 0, "#1012: Studio host class-library JSON smoke should exit successfully");
    expect_contains(class_process.stdout_text, "\"kind\": \"class_library\"",
                    "#1012: class-library JSON should preserve class-library document kind");
    expect_contains(class_process.stdout_text, "\"selectionContext\": \"class_designer\"",
                    "#1012: class-library documents should default to class-designer JSON contexts");
    expect_contains(class_process.stdout_text, "\"builderCount\": 1",
                    "#1012: class-library JSON should expose class builder count");
    expect_contains(class_process.stdout_text, "\"id\": \"class-builder\"",
                    "#1012: class-library JSON should expose class builder ids");
    expect_not_contains(class_process.stdout_text, "\"id\": \"form-builder\"",
                        "#1012: class-library JSON should not expose form builder ids");
    expect_not_contains(class_process.stdout_text, "\"id\": \"control-builder\"",
                        "#1012: class-library JSON should not expose control builder ids");

    const fs::path menu_path = temp_root / "mainmenu.mnx";
    write_synthetic_form_asset(menu_path);
    const auto menu_process = run_process_capture(
        studio_host_path,
        {"--path", menu_path.string(), "--json"},
        temp_root);

    if (menu_process.exit_code != 0) {
        std::cerr << "studio host menu stdout:\n" << menu_process.stdout_text << "\n";
        std::cerr << "studio host menu stderr:\n" << menu_process.stderr_text << "\n";
        std::cerr << "fixture root: " << temp_root << "\n";
    }

    expect(menu_process.exit_code == 0, "#1013: Studio host menu JSON smoke should exit successfully");
    expect_contains(menu_process.stdout_text, "\"kind\": \"menu\"",
                    "#1013: menu JSON should preserve menu document kind");
    expect_contains(menu_process.stdout_text, "\"selectionContext\": \"menu_item\"",
                    "#1013: menu documents should default to menu-item JSON contexts");
    expect_contains(menu_process.stdout_text, "\"builderCount\": 1",
                    "#1013: menu JSON should expose menu builder count");
    expect_contains(menu_process.stdout_text, "\"toolboxItemCount\": 0",
                    "#1013: menu JSON should expose zero toolbox-item count");
    expect_contains(menu_process.stdout_text, "\"id\": \"menu-designer\"",
                    "#1013: menu JSON should expose menu designer builder ids");
    expect_not_contains(menu_process.stdout_text, "\"id\": \"form-builder\"",
                        "#1013: menu JSON should not expose form builder ids");

    const fs::path label_data_environment_path = temp_root / "mailing_data_environment.lbx";
    write_synthetic_table_with_data_environment(label_data_environment_path);
    const auto label_data_environment_process = run_process_capture(
        studio_host_path,
        {"--path", label_data_environment_path.string(), "--record", "0", "--json"},
        temp_root);

    if (label_data_environment_process.exit_code != 0) {
        std::cerr << "studio host label data-environment stdout:\n"
                  << label_data_environment_process.stdout_text << "\n";
        std::cerr << "studio host label data-environment stderr:\n"
                  << label_data_environment_process.stderr_text << "\n";
        std::cerr << "fixture root: " << temp_root << "\n";
    }

    expect(label_data_environment_process.exit_code == 0,
           "#1016: Studio host selected label DataEnvironment JSON smoke should exit successfully");
    expect_contains(label_data_environment_process.stdout_text, "\"kind\": \"label\"",
                    "#1016: selected DataEnvironment label JSON should preserve label document kind");
    expect_contains(label_data_environment_process.stdout_text, "\"selectionContext\": \"data_environment\"",
                    "#1016: selected label DataEnvironment records should infer data-environment JSON contexts");
    expect_contains(label_data_environment_process.stdout_text, "\"toolboxItemCount\": 0",
                    "#1016: selected label DataEnvironment records should expose zero toolbox-item count");
    expect_contains(label_data_environment_process.stdout_text, "\"id\": \"edit-data-environment\"",
                    "#1016: selected label DataEnvironment records should expose data-environment editor actions");
    expect_contains(label_data_environment_process.stdout_text, "\"id\": \"data-environment-builder\"",
                    "#1016: selected label DataEnvironment records should expose data-environment builders");
    expect_not_contains(label_data_environment_process.stdout_text, "\"selectionContext\": \"report_expression\"",
                        "#2074: selected label DataEnvironment records should not expose report-expression contexts");
    expect_not_contains(label_data_environment_process.stdout_text, "\"selectionContext\": \"label_expression\"",
                        "#1016: selected label DataEnvironment records should not keep label-expression defaults");
    expect_not_contains(label_data_environment_process.stdout_text, "\"id\": \"report-builder\"",
                        "#2074: selected label DataEnvironment records should not expose report builder metadata");
    expect_not_contains(label_data_environment_process.stdout_text, "\"id\": \"label-wizard\"",
                        "#2074: selected label DataEnvironment records should not expose label wizard metadata");

    const auto symbol_process = run_process_capture(
        studio_host_path,
        {
            "--path", form_path.string(),
            "--symbol", "cmdSave.Click",
            "--line", "42",
            "--column", "7",
            "--record", "5",
            "--json"
        },
        temp_root);

    if (symbol_process.exit_code != 0) {
        std::cerr << "studio host symbol stdout:\n" << symbol_process.stdout_text << "\n";
        std::cerr << "studio host symbol stderr:\n" << symbol_process.stderr_text << "\n";
        std::cerr << "fixture root: " << temp_root << "\n";
    }

    expect(symbol_process.exit_code == 0, "#963: Studio host symbol-inferred context JSON smoke should exit successfully");
    expect_contains(symbol_process.stdout_text, "\"launchSelection\": {",
                    "#964: Studio host JSON should expose launch selection metadata");
    expect_contains(symbol_process.stdout_text, "\"symbol\": \"cmdSave.Click\"",
                    "#964: Studio host JSON should expose launch selection symbols");
    expect_contains(symbol_process.stdout_text, "\"line\": 42",
                    "#964: Studio host JSON should expose launch selection lines");
    expect_contains(symbol_process.stdout_text, "\"column\": 7",
                    "#964: Studio host JSON should expose launch selection columns");
    expect_contains(symbol_process.stdout_text, "\"recordAvailable\": true",
                    "#967: Studio host JSON should expose explicit launch record availability");
    expect_contains(symbol_process.stdout_text, "\"recordIndex\": 5",
                    "#964: Studio host JSON should expose launch selection record indexes");
    expect_contains(symbol_process.stdout_text, "\"selectedObjectAvailable\": false",
                    "#979: unmatched explicit selected records should report no selected object availability");
    expect_contains(symbol_process.stdout_text, "\"selectedObject\": null",
                    "#967: Studio host JSON should report null selectedObject when no parsed object matches");
    expect_contains(symbol_process.stdout_text, "\"selectionContext\": \"visual_method\"",
                    "#963: method-like launch symbols should infer visual-method JSON contexts");
    expect_contains(symbol_process.stdout_text, "\"id\": \"edit-visual-method\"",
                    "#963: symbol-inferred visual-method contexts should expose method-editor actions");
    expect_not_contains(symbol_process.stdout_text, "\"selectionContext\": \"visual_object\"",
                        "#963: symbol-inferred contexts should replace the form default selection context");

    const auto data_environment_process = run_process_capture(
        studio_host_path,
        {"--path", form_path.string(), "--symbol", "Dataenvironment.OpenTables", "--json"},
        temp_root);

    if (data_environment_process.exit_code != 0) {
        std::cerr << "studio host data-environment stdout:\n" << data_environment_process.stdout_text << "\n";
        std::cerr << "studio host data-environment stderr:\n" << data_environment_process.stderr_text << "\n";
        std::cerr << "fixture root: " << temp_root << "\n";
    }

    expect(data_environment_process.exit_code == 0,
           "#965: Studio host data-environment symbol context JSON smoke should exit successfully");
    expect_contains(data_environment_process.stdout_text, "\"selectionContext\": \"data_environment\"",
                    "#965: DataEnvironment symbols should infer data-environment JSON contexts");
    expect_contains(data_environment_process.stdout_text, "\"builderCount\": 1",
                    "#1009: data-environment JSON should expose designer context builder count");
    expect_contains(data_environment_process.stdout_text, "\"toolboxItemCount\": 0",
                    "#1009: data-environment JSON should expose zero toolbox-item count");
    expect_contains(data_environment_process.stdout_text, "\"id\": \"edit-data-environment\"",
                    "#965: inferred data-environment contexts should expose data-environment editor actions");
    expect_contains(data_environment_process.stdout_text, "\"id\": \"data-environment-builder\"",
                    "#965: inferred data-environment contexts should expose data-environment builders");
    expect_not_contains(data_environment_process.stdout_text, "\"selectionContext\": \"visual_method\"",
                        "#965: DataEnvironment symbols should not fall through to visual-method contexts");

    const auto explicit_precedence_process = run_process_capture(
        studio_host_path,
        {
            "--path", form_path.string(),
            "--symbol", "Dataenvironment.OpenTables",
            "--selection-context", "report_expression",
            "--json"
        },
        temp_root);

    if (explicit_precedence_process.exit_code != 0) {
        std::cerr << "studio host explicit precedence stdout:\n" << explicit_precedence_process.stdout_text << "\n";
        std::cerr << "studio host explicit precedence stderr:\n" << explicit_precedence_process.stderr_text << "\n";
        std::cerr << "fixture root: " << temp_root << "\n";
    }

    expect(explicit_precedence_process.exit_code == 0,
           "#965: Studio host explicit-over-DataEnvironment context JSON smoke should exit successfully");
    expect_contains(explicit_precedence_process.stdout_text, "\"selectionContext\": \"report_expression\"",
                    "#965: explicit selection contexts should serialize when a DataEnvironment symbol is also present");
    expect_not_contains(explicit_precedence_process.stdout_text, "\"selectionContext\": \"data_environment\"",
                        "#965: explicit selection contexts should override symbol-inferred data-environment contexts");

    const fs::path selected_container_path = temp_root / "selected_container.scx";
    write_synthetic_form_table_with_container_object(selected_container_path);
    const auto selected_container_process = run_process_capture(
        studio_host_path,
        {"--path", selected_container_path.string(), "--record", "1", "--json"},
        temp_root);

    if (selected_container_process.exit_code != 0) {
        std::cerr << "studio host selected container stdout:\n" << selected_container_process.stdout_text << "\n";
        std::cerr << "studio host selected container stderr:\n" << selected_container_process.stderr_text << "\n";
        std::cerr << "fixture root: " << temp_root << "\n";
    }

    expect(selected_container_process.exit_code == 0,
           "#1015: Studio host selected container JSON smoke should exit successfully");
    expect_contains(selected_container_process.stdout_text, "\"recordIndex\": 1",
                    "#1015: selected container JSON should preserve selected record index");
    expect_contains(selected_container_process.stdout_text, "\"baseclassName\": \"pageframe\"",
                    "#1015: selected container JSON should expose selected baseclass metadata");
    expect_contains(selected_container_process.stdout_text, "\"selectionContext\": \"container_object\"",
                    "#1015: selected container records should infer container-object JSON contexts");
    expect_contains(selected_container_process.stdout_text, "\"id\": \"control-builder\"",
                    "#1015: inferred container contexts should expose control builders");
    expect_contains(selected_container_process.stdout_text, "\"id\": \"grid-builder\"",
                    "#1015: inferred container contexts should expose grid builders");
    expect_contains(selected_container_process.stdout_text, "\"id\": \"checkbox\"",
                    "#1015: inferred container contexts should expose container-safe toolbox items");
    expect_not_contains(selected_container_process.stdout_text, "\"id\": \"form-builder\"",
                        "#1015: inferred container contexts should not expose form builders");

    const auto selected_grid_process = run_process_capture(
        studio_host_path,
        {"--path", selected_container_path.string(), "--record", "2", "--json"},
        temp_root);

    if (selected_grid_process.exit_code != 0) {
        std::cerr << "studio host selected grid stdout:\n" << selected_grid_process.stdout_text << "\n";
        std::cerr << "studio host selected grid stderr:\n" << selected_grid_process.stderr_text << "\n";
        std::cerr << "fixture root: " << temp_root << "\n";
    }

    expect(selected_grid_process.exit_code == 0,
           "#1015: Studio host selected grid JSON smoke should exit successfully");
    expect_contains(selected_grid_process.stdout_text, "\"baseclassName\": \"grid\"",
                    "#1015: selected grid JSON should expose selected baseclass metadata");
    expect_contains(selected_grid_process.stdout_text, "\"selectionContext\": \"container_object\"",
                    "#1015: selected grid records should infer container-object JSON contexts");

    const fs::path selected_form_path = temp_root / "selected.scx";
    write_synthetic_form_table_with_objects(selected_form_path);
    const auto selected_object_process = run_process_capture(
        studio_host_path,
        {"--path", selected_form_path.string(), "--record", "1", "--json"},
        temp_root);

    if (selected_object_process.exit_code != 0) {
        std::cerr << "studio host selected object stdout:\n" << selected_object_process.stdout_text << "\n";
        std::cerr << "studio host selected object stderr:\n" << selected_object_process.stderr_text << "\n";
        std::cerr << "fixture root: " << temp_root << "\n";
    }

    expect(selected_object_process.exit_code == 0,
           "#967: Studio host selected-object JSON smoke should exit successfully");
    expect_contains(selected_object_process.stdout_text, "\"objectCount\": 4",
                    "#977: Studio host JSON should expose document-level object counts");
    expect_contains(selected_object_process.stdout_text, "\"deletedObjectCount\": 0",
                    "#977: Studio host JSON should expose document-level deleted object counts");
    expect_contains(selected_object_process.stdout_text, "\"rootObjectCount\": 2",
                    "#978: Studio host JSON should expose document-level root object counts");
    expect_contains(selected_object_process.stdout_text, "\"rootRecordIndexes\": [0, 1]",
                    "#981: Studio host JSON should expose document-level root record links");
    expect_contains(selected_object_process.stdout_text, "\"leafObjectCount\": 3",
                    "#982: Studio host JSON should expose document-level leaf object counts");
    expect_contains(selected_object_process.stdout_text, "\"leafRecordIndexes\": [0, 2, 3]",
                    "#982: Studio host JSON should expose document-level leaf record links");
    expect_contains(selected_object_process.stdout_text, "\"maxObjectDepth\": 1",
                    "#983: Studio host JSON should expose document-level maximum object tree depth");
    expect_contains(selected_object_process.stdout_text, "\"selectedObjectAvailable\": true",
                    "#979: matched explicit selected records should report selected object availability");
    expect_contains(selected_object_process.stdout_text, "\"selectedObject\": {",
                    "#967: Studio host JSON should expose selected object summaries for matching records");
    expect_contains(selected_object_process.stdout_text, "\"recordIndex\": 1",
                    "#967: selected object summaries should expose selected record indexes");
    expect_contains(selected_object_process.stdout_text, "\"objectName\": \"frmCustomer\"",
                    "#967: selected object summaries should expose object names");
    expect_contains(selected_object_process.stdout_text, "\"uniqueId\": \"form-1\"",
                    "#967: selected object summaries should expose unique ids");
    expect_contains(selected_object_process.stdout_text, "\"className\": \"customerform\"",
                    "#967: selected object summaries should expose class names");
    expect_contains(selected_object_process.stdout_text, "\"baseclassName\": \"form\"",
                    "#967: selected object summaries should expose baseclass names");
    const auto selected_object_begin = selected_object_process.stdout_text.find("\"selectedObject\": {");
    const auto selected_object_end =
        selected_object_begin == std::string::npos
            ? std::string::npos
            : selected_object_process.stdout_text.find("\"hasSidecar\"", selected_object_begin);
    expect(selected_object_begin != std::string::npos &&
               selected_object_end != std::string::npos &&
               selected_object_end > selected_object_begin,
           "#968: Studio host JSON should delimit a selected-object section before document metadata resumes");
    if (selected_object_begin != std::string::npos &&
        selected_object_end != std::string::npos &&
        selected_object_end > selected_object_begin) {
        const auto selected_object_json =
            selected_object_process.stdout_text.substr(selected_object_begin, selected_object_end - selected_object_begin);
        expect_contains(selected_object_json, "\"deleted\": false",
                        "#974: selected object summaries should expose parsed deletion state");
        expect_contains(selected_object_json, "\"properties\": [",
                        "#968: selected object summaries should expose direct property snapshots");
        expect_contains(selected_object_json, "\"name\": \"OBJNAME\"",
                        "#968: selected object properties should include DBF field names");
        expect_contains(selected_object_json, "\"type\": \"C\"",
                        "#968: selected object properties should preserve DBF field types");
        expect_contains(selected_object_json, "\"isNull\": false",
                        "#968: selected object properties should preserve DBF null flags");
        expect_contains(selected_object_json, "\"value\": \"frmCustomer\"",
                        "#968: selected object properties should include selected object values");
        const auto objname_property_begin = selected_object_json.find("\"name\": \"OBJNAME\"");
        expect(objname_property_begin != std::string::npos,
               "#975: selected object properties should include an OBJNAME property object");
        if (objname_property_begin != std::string::npos) {
            const auto objname_property_end = selected_object_json.find("}", objname_property_begin);
            const auto objname_property_json =
                objname_property_end == std::string::npos
                    ? selected_object_json.substr(objname_property_begin)
                    : selected_object_json.substr(objname_property_begin, objname_property_end - objname_property_begin);
            expect_contains(objname_property_json, "\"fieldIndex\": 3",
                            "#975: selected direct-field properties should expose DBF field indexes");
            expect_contains(objname_property_json, "\"memoBlockNumber\": 0",
                            "#975: selected direct-field properties should expose memo block provenance");
            expect_contains(objname_property_json, "\"derivedFromPropertyBlob\": false",
                            "#975: selected direct-field properties should not be marked as property-blob derived");
            expect_contains(objname_property_json, "\"sourceLineIndex\": null",
                            "#975: selected direct-field properties should expose null source line provenance");
        }
        expect_contains(selected_object_json, "\"name\": \"BASECLASS\"",
                        "#968: selected object properties should include later direct DBF fields");
        expect_contains(selected_object_json, "\"value\": \"form\"",
                        "#968: selected object properties should include selected baseclass values");
        expect_contains(selected_object_json, "\"childCount\": 2",
                        "#970: selected parent object summaries should expose direct child counts");
        expect_contains(selected_object_json, "\"childRecordIndexes\": [2, 3]",
                        "#980: selected parent object summaries should expose direct child record links");
        expect_contains(selected_object_json, "\"parentRecordIndex\": null",
                        "#971: root selected object summaries should expose null parent record links");
        expect_contains(selected_object_json, "\"ancestorRecordIndexes\": []",
                        "#985: root selected object summaries should expose empty ancestor record links");
        expect_contains(selected_object_json, "\"objectPath\": \"frmCustomer\"",
                        "#972: root selected object summaries should expose direct object paths");
        expect_contains(selected_object_json, "\"objectDepth\": 0",
                        "#983: root selected object summaries should expose zero object tree depth");
        expect_contains(selected_object_json, "\"siblingIndex\": 1",
                        "#984: selected root object summaries should expose document-root sibling order");
        expect_contains(selected_object_json, "\"siblingCount\": 2",
                        "#984: selected root object summaries should expose document-root sibling count");
        expect_contains(selected_object_json, "\"objectTypeCode\": 1",
                        "#973: selected object summaries should expose raw object type codes");
        expect_contains(selected_object_json, "\"objectCode\": 0",
                        "#973: selected object summaries should expose raw object codes");
        expect_contains(selected_object_json, "\"platform\": \"WINDOWS\"",
                        "#973: selected object summaries should expose parsed platform metadata");
        expect_contains(selected_object_json, "\"propertyCount\": 8",
                        "#976: selected object summaries should expose direct property counts");
    }
    const auto objects_begin = selected_object_process.stdout_text.find("\"objects\": [");
    expect(objects_begin != std::string::npos,
           "#969: Studio host JSON should expose a full object array section");
    if (objects_begin != std::string::npos) {
        const auto objects_json = selected_object_process.stdout_text.substr(objects_begin);
        expect_contains(objects_json, "\"objectName\": \"frmCustomer\"",
                        "#969: full object entries should expose object names directly");
        expect_contains(objects_json, "\"uniqueId\": \"form-1\"",
                        "#969: full object entries should expose unique ids directly");
        expect_contains(objects_json, "\"parentName\": \"\"",
                        "#969: full object entries should expose parent names directly");
        expect_contains(objects_json, "\"className\": \"customerform\"",
                        "#969: full object entries should expose class names directly");
        expect_contains(objects_json, "\"baseclassName\": \"form\"",
                        "#969: full object entries should expose baseclass names directly");
        const auto child_object_begin = objects_json.find("\"objectName\": \"cmdSave\"");
        expect(child_object_begin != std::string::npos,
               "#970: synthetic SCX object array should include the child control object");
        if (child_object_begin != std::string::npos) {
            const auto child_entry_begin = objects_json.rfind("{", child_object_begin);
            const auto child_properties_begin = objects_json.find("\"properties\": [", child_object_begin);
            const auto child_object_json =
                child_entry_begin == std::string::npos
                    ? objects_json.substr(child_object_begin)
                    : child_properties_begin == std::string::npos
                        ? objects_json.substr(child_entry_begin)
                        : objects_json.substr(child_entry_begin, child_properties_begin - child_entry_begin);
            expect_contains(child_object_json, "\"parentName\": \"frmCustomer\"",
                            "#970: child object entries should expose their parent object name");
            expect_contains(child_object_json, "\"parentRecordIndex\": 1",
                            "#971: child object entries should expose resolved parent record links");
            expect_contains(child_object_json, "\"ancestorRecordIndexes\": [1]",
                            "#985: child object entries should expose root-to-parent ancestor record links");
            expect_contains(child_object_json, "\"childCount\": 0",
                            "#970: leaf child object entries should expose zero child count");
            expect_contains(child_object_json, "\"childRecordIndexes\": []",
                            "#980: leaf child object entries should expose empty child record links");
            expect_contains(child_object_json, "\"objectPath\": \"frmCustomer.cmdSave\"",
                            "#972: child object entries should expose parent-prefixed object paths");
            expect_contains(child_object_json, "\"objectDepth\": 1",
                            "#983: child object entries should expose nested object tree depth");
            expect_contains(child_object_json, "\"siblingIndex\": 0",
                            "#984: first child object entries should expose sibling order");
            expect_contains(child_object_json, "\"siblingCount\": 2",
                            "#984: child object entries should expose sibling count");
            expect_contains(child_object_json, "\"objectTypeCode\": 4",
                            "#973: child object entries should expose raw object type codes");
            expect_contains(child_object_json, "\"objectCode\": 2",
                            "#973: child object entries should expose raw object codes");
            expect_contains(child_object_json, "\"platform\": \"WINDOWS\"",
                            "#973: child object entries should expose parsed platform metadata");
            expect_contains(child_object_json, "\"propertyCount\": 8",
                            "#976: child object entries should expose direct property counts");
        }
        const auto sibling_object_begin = objects_json.find("\"objectName\": \"txtName\"");
        expect(sibling_object_begin != std::string::npos,
               "#984: synthetic SCX object array should include the second sibling control object");
        if (sibling_object_begin != std::string::npos) {
            const auto sibling_entry_begin = objects_json.rfind("{", sibling_object_begin);
            const auto sibling_properties_begin = objects_json.find("\"properties\": [", sibling_object_begin);
            const auto sibling_object_json =
                sibling_entry_begin == std::string::npos
                    ? objects_json.substr(sibling_object_begin)
                    : sibling_properties_begin == std::string::npos
                        ? objects_json.substr(sibling_entry_begin)
                        : objects_json.substr(sibling_entry_begin, sibling_properties_begin - sibling_entry_begin);
            expect_contains(sibling_object_json, "\"parentRecordIndex\": 1",
                            "#984: second child object entries should preserve resolved parent links");
            expect_contains(sibling_object_json, "\"ancestorRecordIndexes\": [1]",
                            "#985: second child object entries should expose root-to-parent ancestor record links");
            expect_contains(sibling_object_json, "\"objectDepth\": 1",
                            "#984: second child object entries should expose nested object tree depth");
            expect_contains(sibling_object_json, "\"siblingIndex\": 1",
                            "#984: second child object entries should expose sibling order");
            expect_contains(sibling_object_json, "\"siblingCount\": 2",
                            "#984: second child object entries should expose sibling count");
        }
    }

    const fs::path invalid_codes_path = temp_root / "invalid_raw_codes.scx";
    write_synthetic_form_table_with_invalid_raw_codes(invalid_codes_path);
    const auto invalid_codes_process = run_process_capture(
        studio_host_path,
        {"--path", invalid_codes_path.string(), "--record", "0", "--json"},
        temp_root);

    if (invalid_codes_process.exit_code != 0) {
        std::cerr << "studio host invalid selected object raw-code stdout:\n"
                  << invalid_codes_process.stdout_text << "\n";
        std::cerr << "studio host invalid selected object raw-code stderr:\n"
                  << invalid_codes_process.stderr_text << "\n";
        std::cerr << "fixture root: " << temp_root << "\n";
    }

    expect(invalid_codes_process.exit_code == 0,
           "#1749: invalid selected-object raw codes should keep Studio host inspection non-failing");
    expect_contains(invalid_codes_process.stdout_text, "\"selectedObjectAvailable\": true",
                    "#1749: invalid raw-code selected records should still resolve selected objects");
    expect_contains(invalid_codes_process.stdout_text, "\"objectCount\": 3",
                    "#1749: invalid raw-code documents should still expose parsed object counts");

    const auto invalid_selected_begin = invalid_codes_process.stdout_text.find("\"selectedObject\": {");
    const auto invalid_selected_end =
        invalid_selected_begin == std::string::npos
            ? std::string::npos
            : invalid_codes_process.stdout_text.find("\"hasSidecar\"", invalid_selected_begin);
    expect(invalid_selected_begin != std::string::npos &&
               invalid_selected_end != std::string::npos &&
               invalid_selected_end > invalid_selected_begin,
           "#1749: invalid raw-code JSON should delimit selected-object metadata");
    if (invalid_selected_begin != std::string::npos &&
        invalid_selected_end != std::string::npos &&
        invalid_selected_end > invalid_selected_begin) {
        const auto selected_json =
            invalid_codes_process.stdout_text.substr(invalid_selected_begin, invalid_selected_end - invalid_selected_begin);
        expect_contains(selected_json, "\"objectName\": \"cmdMalformed\"",
                        "#1749: invalid raw-code selected-object JSON should preserve object identity");
        expect_contains(selected_json, "\"objectTypeCode\": 0",
                        "#1749: malformed selected OBJTYPE text should not fabricate parsed type codes");
        expect_contains(selected_json, "\"objectCode\": 0",
                        "#1749: malformed selected OBJCODE text should not fabricate parsed object codes");
        expect_contains_in_order(
            selected_json,
            {
                "\"name\": \"OBJTYPE\"",
                "\"type\": \"C\"",
                "\"value\": \"type?\"",
                "\"fieldIndex\": 0",
                "\"memoBlockNumber\": 0"
            },
            "#1749: malformed selected OBJTYPE source text should remain inspectable");
        expect_contains_in_order(
            selected_json,
            {
                "\"name\": \"OBJCODE\"",
                "\"type\": \"C\"",
                "\"value\": \"code?\"",
                "\"fieldIndex\": 1",
                "\"memoBlockNumber\": 0"
            },
            "#1749: malformed selected OBJCODE source text should remain inspectable");
    }

    const auto invalid_objects_begin = invalid_codes_process.stdout_text.find("\"objects\": [");
    expect(invalid_objects_begin != std::string::npos,
           "#1749: invalid raw-code JSON should expose the full object array");
    if (invalid_objects_begin != std::string::npos) {
        const auto invalid_objects_json = invalid_codes_process.stdout_text.substr(invalid_objects_begin);
        const auto malformed_begin = invalid_objects_json.find("\"objectName\": \"cmdMalformed\"");
        const auto oversized_begin = invalid_objects_json.find("\"objectName\": \"cmdOversized\"");
        const auto dot_leading_begin = invalid_objects_json.find("\"objectName\": \"cmdDotLeading\"");
        expect(malformed_begin != std::string::npos &&
                   oversized_begin != std::string::npos &&
                   dot_leading_begin != std::string::npos,
               "#1749: invalid raw-code object array should include malformed, oversized, and dot-leading rows");
        if (malformed_begin != std::string::npos) {
            const auto malformed_entry_begin = invalid_objects_json.rfind("{", malformed_begin);
            const auto malformed_json_begin = malformed_entry_begin == std::string::npos
                ? malformed_begin
                : malformed_entry_begin;
            const auto malformed_json = oversized_begin == std::string::npos
                ? invalid_objects_json.substr(malformed_json_begin)
                : invalid_objects_json.substr(malformed_json_begin, oversized_begin - malformed_json_begin);
            expect_contains(malformed_json, "\"objectTypeCode\": 0",
                            "#1749: malformed object-array OBJTYPE text should default parsed codes to zero");
            expect_contains(malformed_json, "\"objectCode\": 0",
                            "#1749: malformed object-array OBJCODE text should default parsed codes to zero");
            expect_contains(malformed_json, "\"value\": \"type?\"",
                            "#1749: malformed object-array OBJTYPE source text should remain inspectable");
            expect_contains(malformed_json, "\"value\": \"code?\"",
                            "#1749: malformed object-array OBJCODE source text should remain inspectable");
        }
        if (oversized_begin != std::string::npos) {
            const auto oversized_entry_begin = invalid_objects_json.rfind("{", oversized_begin);
            const auto oversized_json_begin = oversized_entry_begin == std::string::npos
                ? oversized_begin
                : oversized_entry_begin;
            const auto oversized_json = dot_leading_begin == std::string::npos
                ? invalid_objects_json.substr(oversized_json_begin)
                : invalid_objects_json.substr(oversized_json_begin, dot_leading_begin - oversized_json_begin);
            expect_contains(oversized_json, "\"objectTypeCode\": 0",
                            "#1749: oversized object-array OBJTYPE text should default parsed codes to zero");
            expect_contains(oversized_json, "\"objectCode\": 0",
                            "#1749: oversized object-array OBJCODE text should default parsed codes to zero");
            expect_contains(oversized_json, "\"value\": \"999999999999999999999999999999\"",
                            "#1749: oversized object-array OBJTYPE source text should remain inspectable");
            expect_contains(oversized_json, "\"value\": \"-999999999999999999999999999999\"",
                            "#1749: oversized object-array OBJCODE source text should remain inspectable");
        }
        if (dot_leading_begin != std::string::npos) {
            const auto dot_leading_entry_begin = invalid_objects_json.rfind("{", dot_leading_begin);
            const auto dot_leading_json_begin = dot_leading_entry_begin == std::string::npos
                ? dot_leading_begin
                : dot_leading_entry_begin;
            const auto dot_leading_json = invalid_objects_json.substr(dot_leading_json_begin);
            expect_contains(dot_leading_json, "\"objectTypeCode\": 0",
                            "#1749: dot-leading object-array OBJTYPE text should default parsed codes to zero");
            expect_contains(dot_leading_json, "\"objectCode\": 0",
                            "#1749: dot-leading object-array OBJCODE text should default parsed codes to zero");
            expect_contains(dot_leading_json, "\"value\": \".5\"",
                            "#1749: dot-leading object-array OBJTYPE source text should remain inspectable");
            expect_contains(dot_leading_json, "\"value\": \".7\"",
                            "#1749: dot-leading object-array OBJCODE source text should remain inspectable");
        }
    }

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_exposes_designer_launch_surfaces(const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root = fs::temp_directory_path() / "copperfin_studio_host_designer_launch_surfaces_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);
    ScopedDefaultLocaleCatalogEnvironment default_locale_environment;

    const auto visual_process = run_process_capture(
        studio_host_path,
        {
            "--designer-launch-surfaces",
            "--selection-context", "visual_object",
            "--path", "forms/customer.scx",
            "--record", "1",
            "--object-name", "frmCustomer",
            "--unique-id", "form-guid",
            "--symbol", "Click",
            "--line", "12",
            "--column", "4",
            "--json"
        },
        temp_root);
    expect(visual_process.exit_code == 0,
        "#1212: designer launch-surface JSON should accept visual-object contexts");
    expect_contains(visual_process.stdout_text, "\"designerLaunchSurfaces\": {",
        "#1212: designer launch-surface JSON should expose a plan object");
    expect_contains(visual_process.stdout_text, "\"selectionContext\": \"visual_object\"",
        "#1212: designer launch-surface JSON should expose selected Studio contexts");
    expect_contains(visual_process.stdout_text, "\"assetPath\": \"forms/customer.scx\"",
        "#1212: designer launch-surface JSON should preserve asset paths");
    expect_contains(visual_process.stdout_text, "\"recordIndex\": 1",
        "#1212: designer launch-surface JSON should preserve record indexes");
    expect_contains(visual_process.stdout_text, "\"objectName\": \"frmCustomer\"",
        "#1212: designer launch-surface JSON should preserve object names");
    expect_contains(visual_process.stdout_text, "\"uniqueId\": \"form-guid\"",
        "#1212: designer launch-surface JSON should preserve unique ids");
    expect_contains(visual_process.stdout_text, "\"symbol\": \"Click\"",
        "#1212: designer launch-surface JSON should preserve editor symbols");
    expect_contains(visual_process.stdout_text, "\"line\": 12",
        "#1212: designer launch-surface JSON should preserve editor lines");
    expect_contains(visual_process.stdout_text, "\"column\": 4",
        "#1212: designer launch-surface JSON should preserve editor columns");
    expect_contains(visual_process.stdout_text, "\"launchReadySelectionContexts\": [\"visual_object\"]",
        "#1399: designer launch-surface JSON should summarize launch-ready selected contexts");
    expect_contains(visual_process.stdout_text, "\"launchBlockedSelectionContexts\": []",
        "#1399: designer launch-surface JSON should expose empty blocked selected contexts for ready launches");
    expect_contains(visual_process.stdout_text, "\"launchBlockedErrors\": []",
        "#1399: designer launch-surface JSON should expose empty blocked errors for ready launches");
    expect_contains(visual_process.stdout_text, "\"editorActionLaunchPlanCount\": ",
        "#1212: designer launch-surface JSON should expose action launch-plan counts");
    expect_contains(visual_process.stdout_text, "\"builderLaunchPlanCount\": ",
        "#1212: designer launch-surface JSON should expose builder launch-plan counts");
    expect_contains(visual_process.stdout_text, "\"toolboxAvailable\": true",
        "#1212: designer launch-surface JSON should expose toolbox availability");
    expect_contains(visual_process.stdout_text, "\"actionId\": \"show-property-grid\"",
        "#1212: designer launch-surface JSON should include property-grid action plans");
    expect_contains(visual_process.stdout_text, "\"actionId\": \"show-toolbox\"",
        "#1212: designer launch-surface JSON should include toolbox action plans");
    expect_contains(visual_process.stdout_text, "\"builderId\": \"form-builder\"",
        "#1212: designer launch-surface JSON should include form builder plans");
    expect_contains(visual_process.stdout_text, "\"builderId\": \"grid-builder\"",
        "#1212: designer launch-surface JSON should include control builder plans");
    expect_contains(visual_process.stdout_text, "\"toolboxContext\": \"form\"",
        "#1212: designer launch-surface JSON should include resolved toolbox contexts");
    expect_contains(visual_process.stdout_text, "\"id\": \"textbox\"",
        "#1212: designer launch-surface JSON should include toolbox item descriptors");

    const auto menu_process = run_process_capture(
        studio_host_path,
        {
            "--designer-launch-surfaces",
            "--selection-context", "menu_item",
            "--path", "menus/main.mnx",
            "--record", "5",
            "--object-name", "FileExit",
            "--unique-id", "menu-guid",
            "--json"
        },
        temp_root);
    expect(menu_process.exit_code == 0,
        "#1212: designer launch-surface JSON should keep unsupported toolbox contexts as aggregate successes");
    expect_contains(menu_process.stdout_text, "\"selectionContext\": \"menu_item\"",
        "#1212: menu launch-surface JSON should expose selected Studio contexts");
    expect_contains(menu_process.stdout_text, "\"toolboxAvailable\": false",
        "#1212: menu launch-surface JSON should expose unsupported toolbox availability");
    expect_contains(menu_process.stdout_text, "\"toolboxPaletteLaunchPlan\": null",
        "#1212: menu launch-surface JSON should expose null toolbox plans");
    expect_contains(menu_process.stdout_text, "\"launchReadySelectionContexts\": []",
        "#1399: menu launch-surface JSON should expose empty ready selected contexts");
    expect_contains(menu_process.stdout_text, "\"launchBlockedSelectionContexts\": [\"menu_item\"]",
        "#1399: menu launch-surface JSON should summarize launch-blocked selected contexts");
    expect_contains(menu_process.stdout_text,
        "\"launchBlockedErrors\": [\"The selected Studio context does not expose a toolbox palette.\"]",
        "#1399: menu launch-surface JSON should summarize blocked selected-context reasons");
    expect_contains(menu_process.stdout_text,
        "\"toolboxError\": \"The selected Studio context does not expose a toolbox palette.\"",
        "#1212: menu launch-surface JSON should preserve unsupported toolbox reasons");
    expect_contains(menu_process.stdout_text, "\"actionId\": \"show-property-grid\"",
        "#1212: menu launch-surface JSON should still include supported editor actions");
    expect_contains(menu_process.stdout_text, "\"actionId\": \"edit-menu-command\"",
        "#1413: menu launch-surface JSON should include menu command editor actions");
    expect_contains(menu_process.stdout_text, "\"builderId\": \"menu-designer\"",
        "#1212: menu launch-surface JSON should still include supported builders");

    const auto unknown_context_process = run_process_capture(
        studio_host_path,
        {
            "--designer-launch-surfaces",
            "--selection-context", "unknown",
            "--json"
        },
        temp_root);
    expect(unknown_context_process.exit_code == 2,
        "#1212: designer launch-surface JSON should reject unknown selection contexts");
    expect_contains(unknown_context_process.stdout_text, "\"designerLaunchSurfaces\": null",
        "#1212: unknown context JSON should not expose a plan object");
    expect_contains(unknown_context_process.stdout_text, "Unknown selection context token: unknown",
        "#1212: unknown designer launch-surface context JSON should report parser errors");

    const auto missing_context_process = run_process_capture(
        studio_host_path,
        {
            "--designer-launch-surfaces",
            "--json"
        },
        temp_root);
    expect(missing_context_process.exit_code == 2,
        "#1212: designer launch-surface JSON should reject missing selection contexts");
    expect_contains(missing_context_process.stdout_text, "No selection context was provided.",
        "#1212: missing designer launch-surface context JSON should report parser errors");

    const auto invalid_line_process = run_process_capture(
        studio_host_path,
        {
            "--designer-launch-surfaces",
            "--selection-context", "visual_object",
            "--line", "-1",
            "--json"
        },
        temp_root);
    expect(invalid_line_process.exit_code == 2,
        "#1212: designer launch-surface JSON should reject invalid line values");
    expect_contains(invalid_line_process.stdout_text, "The --line value must be a non-negative integer.",
        "#1212: invalid designer launch-surface line JSON should report parser errors");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_exposes_designer_invocation_admission(const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_designer_invocation_admission_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);
    ScopedDefaultLocaleCatalogEnvironment default_locale_environment;

    const auto visual_process = run_process_capture(
        studio_host_path,
        {
            "--designer-invocation-admission",
            "--selection-context", "visual_object",
            "--path", "forms/customer.scx",
            "--record", "1",
            "--object-name", "frmCustomer",
            "--unique-id", "form-guid",
            "--symbol", "Click",
            "--line", "12",
            "--column", "4",
            "--admit-editor-invocations", "true",
            "--admit-builder-invocations", "true",
            "--admit-toolbox-invocation", "true",
            "--json"
        },
        temp_root);
    expect(visual_process.exit_code == 0,
        "#1222: designer invocation-admission JSON should accept admitted visual-object contexts");
    expect_contains(visual_process.stdout_text, "\"designerInvocationAdmission\": {",
        "#1222: designer invocation-admission JSON should expose a plan object");
    expect_contains(visual_process.stdout_text, "\"selectionContext\": \"visual_object\"",
        "#1222: designer invocation-admission JSON should expose selected Studio contexts");
    expect_contains(visual_process.stdout_text, "\"assetPath\": \"forms/customer.scx\"",
        "#1222: designer invocation-admission JSON should preserve asset paths");
    expect_contains(visual_process.stdout_text, "\"recordIndex\": 1",
        "#1222: designer invocation-admission JSON should preserve record indexes");
    expect_contains(visual_process.stdout_text, "\"objectName\": \"frmCustomer\"",
        "#1222: designer invocation-admission JSON should preserve object names");
    expect_contains(visual_process.stdout_text, "\"uniqueId\": \"form-guid\"",
        "#1222: designer invocation-admission JSON should preserve unique ids");
    expect_contains(visual_process.stdout_text, "\"symbol\": \"Click\"",
        "#1222: designer invocation-admission JSON should preserve editor symbols");
    expect_contains(visual_process.stdout_text, "\"line\": 12",
        "#1222: designer invocation-admission JSON should preserve editor lines");
    expect_contains(visual_process.stdout_text, "\"column\": 4",
        "#1222: designer invocation-admission JSON should preserve editor columns");
    expect_contains(visual_process.stdout_text, "\"admissionOkSelectionContexts\": [\"visual_object\"]",
        "#1400: designer invocation-admission JSON should summarize admission-clean selected contexts");
    expect_contains(visual_process.stdout_text, "\"admissionBlockedSelectionContexts\": []",
        "#1400: designer invocation-admission JSON should expose empty blocked selected contexts for clean admission");
    expect_contains(visual_process.stdout_text, "\"admissionBlockedErrors\": []",
        "#1400: designer invocation-admission JSON should expose empty blocked errors for clean admission");
    expect_contains(visual_process.stdout_text, "\"editorActionInvocationCount\": ",
        "#1222: designer invocation-admission JSON should expose editor invocation counts");
    expect_contains(visual_process.stdout_text, "\"builderInvocationCount\": ",
        "#1222: designer invocation-admission JSON should expose builder invocation counts");
    expect_contains(visual_process.stdout_text, "\"toolboxAvailable\": true",
        "#1222: designer invocation-admission JSON should expose toolbox availability");
    expect_contains(visual_process.stdout_text, "\"toolboxItemCount\": ",
        "#1222: designer invocation-admission JSON should expose toolbox item counts");
    expect_contains(visual_process.stdout_text, "\"actionId\": \"edit-visual-method\"",
        "#1222: designer invocation-admission JSON should include admitted editor actions");
    expect_contains(visual_process.stdout_text, "\"builderId\": \"form-builder\"",
        "#1222: designer invocation-admission JSON should include admitted builders");
    expect_contains(visual_process.stdout_text, "\"editorInvocationAdmitted\": true",
        "#1222: admitted designer invocation-admission JSON should admit editor invocations");
    expect_contains(visual_process.stdout_text, "\"uiLaunchAdmitted\": true",
        "#1222: admitted designer invocation-admission JSON should admit builder invocations");
    expect_contains(visual_process.stdout_text, "\"paletteInvocationAdmitted\": true",
        "#1222: admitted designer invocation-admission JSON should admit toolbox invocation");
    expect_contains(visual_process.stdout_text, "\"dryRun\": false",
        "#1222: admitted designer invocation-admission JSON should not be aggregate dry-run");
    expect_contains(visual_process.stdout_text, "\"mutatesAsset\": false",
        "#1222: designer invocation-admission JSON should remain non-mutating");

    const auto menu_process = run_process_capture(
        studio_host_path,
        {
            "--designer-invocation-admission",
            "--selection-context", "menu_item",
            "--path", "menus/main.mnx",
            "--record", "5",
            "--object-name", "FileExit",
            "--unique-id", "menu-guid",
            "--json"
        },
        temp_root);
    expect(menu_process.exit_code == 0,
        "#1222: designer invocation-admission JSON should keep unsupported toolbox contexts as aggregate successes");
    expect_contains(menu_process.stdout_text, "\"selectionContext\": \"menu_item\"",
        "#1222: menu designer invocation-admission JSON should expose selected Studio contexts");
    expect_contains(menu_process.stdout_text, "\"toolboxAvailable\": false",
        "#1222: menu designer invocation-admission JSON should expose unsupported toolbox availability");
    expect_contains(menu_process.stdout_text, "\"toolboxInvocation\": null",
        "#1222: menu designer invocation-admission JSON should expose null toolbox invocation");
    expect_contains(menu_process.stdout_text, "\"admissionOkSelectionContexts\": []",
        "#1400: menu invocation-admission JSON should expose empty clean selected contexts");
    expect_contains(menu_process.stdout_text, "\"admissionBlockedSelectionContexts\": [\"menu_item\"]",
        "#1400: menu invocation-admission JSON should summarize admission-blocked selected contexts");
    expect_contains(menu_process.stdout_text,
        "\"admissionBlockedErrors\": [\"The selected Studio context does not expose a toolbox palette.\"]",
        "#1400: menu invocation-admission JSON should summarize blocked selected-context reasons");
    expect_contains(menu_process.stdout_text,
        "\"toolboxError\": \"The selected Studio context does not expose a toolbox palette.\"",
        "#1222: menu designer invocation-admission JSON should preserve unsupported toolbox reasons");
    expect_contains(menu_process.stdout_text, "\"actionId\": \"show-property-grid\"",
        "#1222: menu designer invocation-admission JSON should still include editor actions");
    expect_contains(menu_process.stdout_text, "\"actionId\": \"edit-menu-command\"",
        "#1413: menu designer invocation-admission JSON should include menu command editor actions");
    expect_contains(menu_process.stdout_text, "\"builderId\": \"menu-designer\"",
        "#1222: menu designer invocation-admission JSON should still include builders");
    expect_contains(menu_process.stdout_text, "\"editorInvocationAdmitted\": false",
        "#1222: default designer invocation-admission JSON should not admit editor invocations");
    expect_contains(menu_process.stdout_text, "\"uiLaunchAdmitted\": false",
        "#1222: default designer invocation-admission JSON should not admit builder invocations");
    expect_contains(menu_process.stdout_text, "\"dryRun\": true",
        "#1222: default designer invocation-admission JSON should expose dry-run state");

    const auto invalid_boolean_process = run_process_capture(
        studio_host_path,
        {
            "--designer-invocation-admission",
            "--selection-context", "visual_object",
            "--admit-editor-invocations", "maybe",
            "--json"
        },
        temp_root);
    expect(invalid_boolean_process.exit_code == 2,
        "#1222: designer invocation-admission JSON should reject invalid editor admission booleans");
    expect_contains(invalid_boolean_process.stdout_text,
        "The --admit-editor-invocations value must be true or false.",
        "#1222: invalid designer invocation-admission boolean JSON should report parser errors");

    const auto unknown_context_process = run_process_capture(
        studio_host_path,
        {
            "--designer-invocation-admission",
            "--selection-context", "unknown",
            "--json"
        },
        temp_root);
    expect(unknown_context_process.exit_code == 2,
        "#1222: designer invocation-admission JSON should reject unknown selection contexts");
    expect_contains(unknown_context_process.stdout_text, "\"designerInvocationAdmission\": null",
        "#1222: unknown designer invocation-admission context JSON should not expose a plan object");
    expect_contains(unknown_context_process.stdout_text, "Unknown selection context token: unknown",
        "#1222: unknown designer invocation-admission context JSON should report parser errors");

    const auto invalid_line_process = run_process_capture(
        studio_host_path,
        {
            "--designer-invocation-admission",
            "--selection-context", "visual_object",
            "--line", "-1",
            "--json"
        },
        temp_root);
    expect(invalid_line_process.exit_code == 2,
        "#1222: designer invocation-admission JSON should reject invalid line values");
    expect_contains(invalid_line_process.stdout_text, "The --line value must be a non-negative integer.",
        "#1222: invalid designer invocation-admission line JSON should report parser errors");

    const auto missing_context_process = run_process_capture(
        studio_host_path,
        {
            "--designer-invocation-admission",
            "--json"
        },
        temp_root);
    expect(missing_context_process.exit_code == 2,
        "#1222: designer invocation-admission JSON should reject missing selection contexts");
    expect_contains(missing_context_process.stdout_text, "No selection context was provided.",
        "#1222: missing designer invocation-admission context JSON should report parser errors");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_exposes_designer_dispatch(const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root = fs::temp_directory_path() / "copperfin_studio_host_designer_dispatch_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);
    ScopedDefaultLocaleCatalogEnvironment default_locale_environment;

    const auto visual_process = run_process_capture(
        studio_host_path,
        {
            "--designer-dispatch",
            "--selection-context", "visual_object",
            "--path", "forms/customer.scx",
            "--record", "1",
            "--object-name", "frmCustomer",
            "--unique-id", "form-guid",
            "--symbol", "Click",
            "--line", "12",
            "--column", "4",
            "--admit-editor-invocations", "true",
            "--admit-builder-invocations", "true",
            "--admit-toolbox-invocation", "true",
            "--json"
        },
        temp_root);
    expect(visual_process.exit_code == 0,
        "#1238: designer dispatch JSON should accept admitted visual-object contexts");
    expect_contains(visual_process.stdout_text, "\"designerDispatch\": {",
        "#1238: designer dispatch JSON should expose a dispatch object");
    expect_contains(visual_process.stdout_text, "\"selectionContext\": \"visual_object\"",
        "#1238: designer dispatch JSON should expose selected Studio contexts");
    expect_contains(visual_process.stdout_text, "\"assetPath\": \"forms/customer.scx\"",
        "#1238: designer dispatch JSON should preserve asset paths");
    expect_contains(visual_process.stdout_text, "\"recordIndex\": 1",
        "#1238: designer dispatch JSON should preserve record indexes");
    expect_contains(visual_process.stdout_text, "\"objectName\": \"frmCustomer\"",
        "#1238: designer dispatch JSON should preserve object names");
    expect_contains(visual_process.stdout_text, "\"uniqueId\": \"form-guid\"",
        "#1238: designer dispatch JSON should preserve unique ids");
    expect_contains(visual_process.stdout_text, "\"symbol\": \"Click\"",
        "#1238: designer dispatch JSON should preserve editor symbols");
    expect_contains(visual_process.stdout_text, "\"line\": 12",
        "#1238: designer dispatch JSON should preserve editor lines");
    expect_contains(visual_process.stdout_text, "\"column\": 4",
        "#1238: designer dispatch JSON should preserve editor columns");
    expect_contains(visual_process.stdout_text, "\"dispatchOkSelectionContexts\": [\"visual_object\"]",
        "#1401: designer dispatch JSON should summarize dispatch-clean selected contexts");
    expect_contains(visual_process.stdout_text, "\"dispatchBlockedSelectionContexts\": []",
        "#1401: designer dispatch JSON should expose empty blocked selected contexts for clean dispatch");
    expect_contains(visual_process.stdout_text, "\"dispatchBlockedErrors\": []",
        "#1401: designer dispatch JSON should expose empty blocked errors for clean dispatch");
    expect_contains(visual_process.stdout_text, "\"editorActionDispatchCount\": ",
        "#1238: designer dispatch JSON should expose editor dispatch counts");
    expect_contains(visual_process.stdout_text, "\"builderDispatchCount\": ",
        "#1238: designer dispatch JSON should expose builder dispatch counts");
    expect_contains(visual_process.stdout_text, "\"toolboxDispatchCount\": 1",
        "#1238: designer dispatch JSON should expose toolbox dispatch counts");
    expect_contains(visual_process.stdout_text, "\"dispatchCount\": ",
        "#1238: designer dispatch JSON should expose aggregate dispatch counts");
    expect_contains(visual_process.stdout_text, "\"errorCount\": 0",
        "#1238: admitted designer dispatch JSON should expose zero aggregate errors");
    expect_contains(visual_process.stdout_text, "\"actionId\": \"edit-visual-method\"",
        "#1238: designer dispatch JSON should include editor action dispatches");
    expect_contains(visual_process.stdout_text, "\"builderId\": \"form-builder\"",
        "#1238: designer dispatch JSON should include builder dispatches");
    expect_contains(visual_process.stdout_text, "\"toolboxContext\": \"form\"",
        "#1238: designer dispatch JSON should include toolbox dispatches");
    expect_contains(visual_process.stdout_text, "\"dispatchAdmitted\": true",
        "#1238: admitted designer dispatch JSON should expose dispatch-admitted surfaces");
    expect_contains(visual_process.stdout_text, "\"dryRun\": false",
        "#1238: admitted designer dispatch JSON should not be aggregate dry-run");
    expect_contains(visual_process.stdout_text, "\"executed\": false",
        "#1238: designer dispatch JSON should not execute UI");
    expect_contains(visual_process.stdout_text, "\"mutatesAsset\": false",
        "#1238: designer dispatch JSON should remain non-mutating");

    const auto menu_process = run_process_capture(
        studio_host_path,
        {
            "--designer-dispatch",
            "--selection-context", "menu_item",
            "--path", "menus/main.mnx",
            "--record", "5",
            "--object-name", "FileExit",
            "--unique-id", "menu-guid",
            "--json"
        },
        temp_root);
    expect(menu_process.exit_code == 0,
        "#1238: designer dispatch JSON should keep dry-run menu contexts as aggregate successes");
    expect_contains(menu_process.stdout_text, "\"selectionContext\": \"menu_item\"",
        "#1238: menu designer dispatch JSON should expose selected Studio contexts");
    expect_contains(menu_process.stdout_text, "\"dispatchCount\": 0",
        "#1238: default designer dispatch JSON should expose zero aggregate dispatches");
    expect_contains(menu_process.stdout_text, "\"errorCount\": 5",
        "#1238: default menu designer dispatch JSON should expose per-surface dispatch errors");
    expect_contains(menu_process.stdout_text, "\"dispatchOkSelectionContexts\": []",
        "#1401: default menu dispatch JSON should expose empty clean selected contexts");
    expect_contains(menu_process.stdout_text, "\"dispatchBlockedSelectionContexts\": [\"menu_item\"]",
        "#1401: default menu dispatch JSON should summarize dispatch-blocked selected contexts");
    expect_contains(menu_process.stdout_text,
        "\"dispatchBlockedErrors\": [\"An editor action dispatch request requires an admitted non-dry-run invocation.\"]",
        "#1401: default menu dispatch JSON should summarize first blocked selected-context reason");
    expect_contains(menu_process.stdout_text, "\"dispatchOk\": false",
        "#1238: default menu designer dispatch JSON should expose rejected dispatches");
    expect_contains(menu_process.stdout_text,
        "An editor action dispatch request requires an admitted non-dry-run invocation.",
        "#1238: default menu designer dispatch JSON should report editor dispatch errors");
    expect_contains(menu_process.stdout_text,
        "A builder dispatch request requires an admitted non-dry-run invocation.",
        "#1238: default menu designer dispatch JSON should report builder dispatch errors");
    expect_contains(menu_process.stdout_text,
        "The selected Studio context does not expose a toolbox palette.",
        "#1238: menu designer dispatch JSON should preserve unsupported toolbox reasons");
    expect_contains(menu_process.stdout_text, "\"dryRun\": true",
        "#1238: default designer dispatch JSON should expose dry-run state");

    const auto invalid_boolean_process = run_process_capture(
        studio_host_path,
        {
            "--designer-dispatch",
            "--selection-context", "visual_object",
            "--admit-builder-invocations", "maybe",
            "--json"
        },
        temp_root);
    expect(invalid_boolean_process.exit_code == 2,
        "#1238: designer dispatch JSON should reject invalid builder admission booleans");
    expect_contains(invalid_boolean_process.stdout_text, "\"designerDispatch\": null",
        "#1238: invalid designer dispatch boolean JSON should not expose a dispatch object");
    expect_contains(invalid_boolean_process.stdout_text,
        "The --admit-builder-invocations value must be true or false.",
        "#1238: invalid designer dispatch boolean JSON should report parser errors");

    const auto unknown_context_process = run_process_capture(
        studio_host_path,
        {
            "--designer-dispatch",
            "--selection-context", "unknown",
            "--json"
        },
        temp_root);
    expect(unknown_context_process.exit_code == 2,
        "#1238: designer dispatch JSON should reject unknown selection contexts");
    expect_contains(unknown_context_process.stdout_text, "Unknown selection context token: unknown",
        "#1238: unknown designer dispatch context JSON should report parser errors");

    const auto invalid_line_process = run_process_capture(
        studio_host_path,
        {
            "--designer-dispatch",
            "--selection-context", "visual_object",
            "--line", "-1",
            "--json"
        },
        temp_root);
    expect(invalid_line_process.exit_code == 2,
        "#1238: designer dispatch JSON should reject invalid line values");
    expect_contains(invalid_line_process.stdout_text, "The --line value must be a non-negative integer.",
        "#1238: invalid designer dispatch line JSON should report parser errors");

    const auto missing_context_process = run_process_capture(
        studio_host_path,
        {
            "--designer-dispatch",
            "--json"
        },
        temp_root);
    expect(missing_context_process.exit_code == 2,
        "#1238: designer dispatch JSON should reject missing selection contexts");
    expect_contains(missing_context_process.stdout_text, "No selection context was provided.",
        "#1238: missing designer dispatch context JSON should report parser errors");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_exposes_designer_execution(const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root = fs::temp_directory_path() / "copperfin_studio_host_designer_execution_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);
    ScopedDefaultLocaleCatalogEnvironment default_locale_environment;

    const auto visual_process = run_process_capture(
        studio_host_path,
        {
            "--designer-execute",
            "--selection-context", "visual_object",
            "--path", "forms/customer.scx",
            "--record", "1",
            "--object-name", "frmCustomer",
            "--unique-id", "form-guid",
            "--symbol", "Click",
            "--line", "12",
            "--column", "4",
            "--admit-editor-invocations", "true",
            "--admit-builder-invocations", "true",
            "--admit-toolbox-invocation", "true",
            "--admit-designer-execution", "true",
            "--editor-action-launch-command", COPPERFIN_TEST_SUCCESS_COMMAND,
            "--builder-launch-command", COPPERFIN_TEST_SUCCESS_COMMAND,
            "--toolbox-launch-command", COPPERFIN_TEST_SUCCESS_COMMAND,
            "--json"
        },
        temp_root);
    expect(visual_process.exit_code == 0,
        "#1325: designer execution JSON should accept admitted visual-object contexts");
    expect_contains(visual_process.stdout_text, "\"designerExecution\": {",
        "#1325: designer execution JSON should expose an execution object");
    expect_contains(visual_process.stdout_text, "\"selectionContext\": \"visual_object\"",
        "#1325: designer execution JSON should expose selected Studio contexts");
    expect_contains(visual_process.stdout_text, "\"assetPath\": \"forms/customer.scx\"",
        "#1325: designer execution JSON should preserve asset paths");
    expect_contains(visual_process.stdout_text, "\"executionAdmitted\": true",
        "#1325: designer execution JSON should expose execution admission");
    expect_contains(visual_process.stdout_text, "\"executed\": true",
        "#1325: designer execution JSON should mark aggregate execution complete");
    expect_contains(visual_process.stdout_text, "\"dryRun\": false",
        "#1325: admitted designer execution JSON should not be dry-run");
    expect_contains(visual_process.stdout_text, "\"executionCount\": ",
        "#1325: designer execution JSON should expose execution counts");
    expect_contains(visual_process.stdout_text, "\"errorCount\": 0",
        "#1325: admitted designer execution JSON should expose zero execution errors");
    expect_contains(visual_process.stdout_text, "\"executionReadySelectionContexts\": [\"visual_object\"]",
        "#1402: admitted designer execution JSON should summarize execution-ready selected contexts");
    expect_contains(visual_process.stdout_text, "\"executionBlockedSelectionContexts\": []",
        "#1402: admitted designer execution JSON should expose empty blocked selected contexts");
    expect_contains(visual_process.stdout_text, "\"executionBlockedErrors\": []",
        "#1402: admitted designer execution JSON should expose empty blocked execution errors");
    expect_contains(visual_process.stdout_text, "\"editorActionExecutionCount\": 5",
        "#1342: admitted designer execution JSON should expose editor action execution counts");
    expect_contains(visual_process.stdout_text, "\"builderExecutionCount\": 3",
        "#1342: admitted designer execution JSON should expose builder execution counts");
    expect_contains(visual_process.stdout_text, "\"toolboxExecutionCount\": 1",
        "#1342: admitted designer execution JSON should expose toolbox execution counts");
    expect_contains(visual_process.stdout_text, "\"editorActionExecutedCount\": 5",
        "#1344: admitted designer execution JSON should expose editor action executed counts");
    expect_contains(visual_process.stdout_text, "\"builderExecutedCount\": 3",
        "#1344: admitted designer execution JSON should expose builder executed counts");
    expect_contains(visual_process.stdout_text, "\"toolboxExecutedCount\": 1",
        "#1344: admitted designer execution JSON should expose toolbox executed counts");
    expect_contains(visual_process.stdout_text, "\"editorActionErrorCount\": 0",
        "#1343: admitted designer execution JSON should expose zero editor action errors");
    expect_contains(visual_process.stdout_text, "\"builderErrorCount\": 0",
        "#1343: admitted designer execution JSON should expose zero builder errors");
    expect_contains(visual_process.stdout_text, "\"toolboxErrorCount\": 0",
        "#1343: admitted designer execution JSON should expose zero toolbox errors");
    expect_contains(visual_process.stdout_text, "\"failedEditorActionIds\": []",
        "#1345: admitted designer execution JSON should expose no failed editor actions");
    expect_contains(visual_process.stdout_text, "\"failedEditorActionCommandTokens\": []",
        "#1353: admitted designer execution JSON should expose no failed editor action command tokens");
    expect_contains(visual_process.stdout_text, "\"failedEditorActionExecutedCommands\": []",
        "#1354: admitted designer execution JSON should expose no failed editor action executed commands");
    expect_contains(visual_process.stdout_text, "\"failedEditorActionExitCodes\": []",
        "#1355: admitted designer execution JSON should expose no failed editor action exit codes");
    expect_contains(visual_process.stdout_text, "\"failedEditorActionErrors\": []",
        "#1346: admitted designer execution JSON should expose no failed editor action errors");
    expect_contains(visual_process.stdout_text, "\"failedBuilderIds\": []",
        "#1345: admitted designer execution JSON should expose no failed builders");
    expect_contains(visual_process.stdout_text, "\"failedBuilderCommandTokens\": []",
        "#1353: admitted designer execution JSON should expose no failed builder command tokens");
    expect_contains(visual_process.stdout_text, "\"failedBuilderExecutedCommands\": []",
        "#1354: admitted designer execution JSON should expose no failed builder executed commands");
    expect_contains(visual_process.stdout_text, "\"failedBuilderExitCodes\": []",
        "#1355: admitted designer execution JSON should expose no failed builder exit codes");
    expect_contains(visual_process.stdout_text, "\"failedBuilderErrors\": []",
        "#1346: admitted designer execution JSON should expose no failed builder errors");
    expect_contains(visual_process.stdout_text, "\"toolboxFailed\": false",
        "#1345: admitted designer execution JSON should expose successful toolbox summary state");
    expect_contains(visual_process.stdout_text, "\"toolboxExitCode\": 0",
        "#1355: admitted designer execution JSON should expose successful toolbox exit codes");
    expect_contains(visual_process.stdout_text, "\"toolboxError\": \"\"",
        "#1346: admitted designer execution JSON should expose empty toolbox error summaries");
    expect_contains(visual_process.stdout_text,
        "\"editorActionLaunchCommand\": \"" COPPERFIN_TEST_SUCCESS_COMMAND "\"",
        "#1325: designer execution JSON should expose editor launch commands");
    expect_contains(visual_process.stdout_text,
        "\"builderLaunchCommand\": \"" COPPERFIN_TEST_SUCCESS_COMMAND "\"",
        "#1325: designer execution JSON should expose builder launch commands");
    expect_contains(visual_process.stdout_text,
        "\"toolboxLaunchCommand\": \"" COPPERFIN_TEST_SUCCESS_COMMAND "\"",
        "#1325: designer execution JSON should expose toolbox launch commands");
    expect_contains(visual_process.stdout_text, "\"editorActionExecutions\": [",
        "#1325: designer execution JSON should expose editor execution results");
    expect_contains(visual_process.stdout_text, "\"actionId\": \"edit-visual-method\"",
        "#1337: designer execution JSON should expose editor child action identities");
    const auto editor_child_begin = visual_process.stdout_text.find("\"actionId\": \"edit-visual-method\"");
    expect(editor_child_begin != std::string::npos,
        "#1349: designer execution JSON should expose an editor child entry for target metadata checks");
    if (editor_child_begin != std::string::npos) {
        const auto editor_child_json = visual_process.stdout_text.substr(editor_child_begin, 900);
        expect_contains(editor_child_json, "\"label\": \"Edit Method\"",
            "#1351: aggregate editor child JSON should expose action labels");
        expect_contains(editor_child_json, "\"kind\": \"source_editor\"",
            "#1351: aggregate editor child JSON should expose action kinds");
        expect_contains(editor_child_json,
            "\"description\": \"Open the selected visual object's PROCEDURE/FUNCTION source in a method editor.\"",
            "#1352: aggregate editor child JSON should expose action descriptions");
        expect_contains(editor_child_json, "\"targetSurface\": \"method-editor\"",
            "#1349: aggregate editor child JSON should expose target surfaces");
        expect_contains(editor_child_json, "\"assetPath\": \"forms/customer.scx\"",
            "#1349: aggregate editor child JSON should expose asset paths");
        expect_contains(editor_child_json, "\"recordIndex\": 1",
            "#1349: aggregate editor child JSON should expose record indexes");
        expect_contains(editor_child_json, "\"objectName\": \"frmCustomer\"",
            "#1349: aggregate editor child JSON should expose object names");
        expect_contains(editor_child_json, "\"uniqueId\": \"form-guid\"",
            "#1349: aggregate editor child JSON should expose unique ids");
        expect_contains(editor_child_json, "\"symbol\": \"Click\"",
            "#1349: aggregate editor child JSON should expose symbols");
        expect_contains(editor_child_json, "\"line\": 12",
            "#1349: aggregate editor child JSON should expose source lines");
        expect_contains(editor_child_json, "\"column\": 4",
            "#1349: aggregate editor child JSON should expose source columns");
    }
    expect_contains(visual_process.stdout_text, "\"commandToken\": \"studio.method_editor.open\"",
        "#1337: designer execution JSON should expose editor child command tokens");
    expect_contains(visual_process.stdout_text, "\"dispatchArguments\": [\"--command-token\", \"studio.method_editor.open\"",
        "#1338: designer execution JSON should expose editor child dispatch arguments");
    expect_contains(visual_process.stdout_text, "\"launchCommand\": \"" COPPERFIN_TEST_SUCCESS_COMMAND "\"",
        "#1348: admitted designer execution JSON should expose child launch commands");
    expect_contains(visual_process.stdout_text, "\"executionAdmitted\": true",
        "#1339: designer execution JSON should expose child execution admission state");
    expect_contains(visual_process.stdout_text, "\"launched\": true",
        "#1339: designer execution JSON should expose child launch state");
    expect_contains(visual_process.stdout_text,
        "\"executedCommand\": \"" + expected_json_shell_command(
            COPPERFIN_TEST_SUCCESS_COMMAND,
            {"--command-token", "studio.method_editor.open"}),
        "#1335: designer execution JSON should expose editor child executed commands");
    expect_contains(visual_process.stdout_text, "\"builderExecutions\": [",
        "#1325: designer execution JSON should expose builder execution results");
    expect_contains(visual_process.stdout_text, "\"builderId\": \"form-builder\"",
        "#1337: designer execution JSON should expose builder child identities");
    const auto builder_child_begin = visual_process.stdout_text.find("\"builderId\": \"form-builder\"");
    expect(builder_child_begin != std::string::npos,
        "#1349: designer execution JSON should expose a builder child entry for target metadata checks");
    if (builder_child_begin != std::string::npos) {
        const auto builder_child_json = visual_process.stdout_text.substr(builder_child_begin, 700);
        expect_contains(builder_child_json, "\"title\": \"Form Builder\"",
            "#1351: aggregate builder child JSON should expose builder titles");
        expect_contains(builder_child_json, "\"kind\": \"builder\"",
            "#1351: aggregate builder child JSON should expose builder kinds");
        expect_contains(builder_child_json, "\"vfp9Equivalent\": \"builder.app form builder\"",
            "#1352: aggregate builder child JSON should expose VFP equivalent metadata");
        expect_contains(builder_child_json, "\"vfp9EquivalentDisplay\": \"builder.app form builder\"",
            "#4303: aggregate builder child JSON should expose localized VFP equivalent display metadata");
        expect_contains(builder_child_json, "\"copperfinComponent\": \"cf_form_surface\"",
            "#1352: aggregate builder child JSON should expose Copperfin component metadata");
        expect_contains(builder_child_json,
            "\"description\": \"Configure form-level data, layout, and generated method defaults.\"",
            "#1352: aggregate builder child JSON should expose builder descriptions");
        expect_contains(builder_child_json, "\"entryPoint\": \"cf_builders.form_builder\"",
            "#1349: aggregate builder child JSON should expose entry points");
        expect_contains(builder_child_json, "\"assetPath\": \"forms/customer.scx\"",
            "#1349: aggregate builder child JSON should expose asset paths");
        expect_contains(builder_child_json, "\"recordIndex\": 1",
            "#1349: aggregate builder child JSON should expose record indexes");
        expect_contains(builder_child_json, "\"objectName\": \"frmCustomer\"",
            "#1349: aggregate builder child JSON should expose object names");
        expect_contains(builder_child_json, "\"uniqueId\": \"form-guid\"",
            "#1349: aggregate builder child JSON should expose unique ids");
    }
    expect_contains(visual_process.stdout_text, "\"commandToken\": \"studio.builder.invoke\"",
        "#1337: designer execution JSON should expose builder child command tokens");
    expect_contains(visual_process.stdout_text, "\"dispatchArguments\": [\"--command-token\", \"studio.builder.invoke\"",
        "#1338: designer execution JSON should expose builder child dispatch arguments");
    expect_contains(visual_process.stdout_text,
        "\"executedCommand\": \"" + expected_json_shell_command(
            COPPERFIN_TEST_SUCCESS_COMMAND,
            {"--command-token", "studio.builder.invoke"}),
        "#1335: designer execution JSON should expose builder child executed commands");
    expect_contains(visual_process.stdout_text, "\"toolboxExecution\": {",
        "#1325: designer execution JSON should expose toolbox execution results");
    const auto toolbox_child_begin = visual_process.stdout_text.find("\"toolboxExecution\": {");
    expect(toolbox_child_begin != std::string::npos,
        "#1349: designer execution JSON should expose a toolbox child entry for target metadata checks");
    if (toolbox_child_begin != std::string::npos) {
        const auto toolbox_child_json = visual_process.stdout_text.substr(toolbox_child_begin, 1600);
        expect_contains(toolbox_child_json, "\"assetPath\": \"forms/customer.scx\"",
            "#1349: aggregate toolbox child JSON should expose asset paths");
        expect_contains(toolbox_child_json, "\"recordIndex\": 1",
            "#1349: aggregate toolbox child JSON should expose record indexes");
        expect_contains(toolbox_child_json, "\"objectName\": \"frmCustomer\"",
            "#1349: aggregate toolbox child JSON should expose object names");
        expect_contains(toolbox_child_json, "\"uniqueId\": \"form-guid\"",
            "#1349: aggregate toolbox child JSON should expose unique ids");
        expect_contains(toolbox_child_json, "\"items\": [",
            "#1350: aggregate toolbox child JSON should expose toolbox item descriptors");
        expect_contains(toolbox_child_json, "\"id\": \"textbox\"",
            "#1350: aggregate toolbox child JSON should include form-safe TextBox items");
        expect_contains(toolbox_child_json, "\"baseClass\": \"TextBox\"",
            "#1350: aggregate toolbox child JSON should expose toolbox item base classes");
    }
    expect_contains(visual_process.stdout_text, "\"toolboxContext\": \"form\"",
        "#1337: designer execution JSON should expose toolbox child contexts");
    expect_contains(visual_process.stdout_text, "\"commandToken\": \"studio.toolbox.palette.invoke\"",
        "#1337: designer execution JSON should expose toolbox child command tokens");
    expect_contains(visual_process.stdout_text,
        "\"dispatchArguments\": [\"--command-token\", \"studio.toolbox.palette.invoke\"",
        "#1338: designer execution JSON should expose toolbox child dispatch arguments");
    expect_contains(visual_process.stdout_text, "\"dryRun\": false",
        "#1339: designer execution JSON should expose child dry-run state");
    expect_contains(visual_process.stdout_text, "\"mutatesAsset\": false",
        "#1339: designer execution JSON should expose child mutation state");
    expect_contains(visual_process.stdout_text,
        "\"executedCommand\": \"" + expected_json_shell_command(
            COPPERFIN_TEST_SUCCESS_COMMAND,
            {"--command-token", "studio.toolbox.palette.invoke"}),
        "#1335: designer execution JSON should expose toolbox child executed commands");

    const auto missing_builder_command_process = run_process_capture(
        studio_host_path,
        {
            "--designer-execute",
            "--selection-context", "visual_object",
            "--admit-builder-invocations", "true",
            "--admit-designer-execution", "true",
            "--json"
        },
        temp_root);
    expect(missing_builder_command_process.exit_code == 2,
        "#1325: designer execution JSON should reject missing required child launch commands");
    expect_contains(missing_builder_command_process.stdout_text,
        "No designer builder launch command was provided.",
        "#1325: missing designer builder launch command JSON should report parser errors");

    const auto dry_run_process = run_process_capture(
        studio_host_path,
        {
            "--designer-execute",
            "--selection-context", "visual_object",
            "--admit-designer-execution", "true",
            "--json"
        },
        temp_root);
    expect(dry_run_process.exit_code == 4,
        "#1325: designer execution JSON should reject aggregate dry-run dispatches");
    expect_contains(dry_run_process.stdout_text, "\"designerExecution\": null",
        "#1325: dry-run designer execution JSON should not expose a result object");
    expect_contains(dry_run_process.stdout_text,
        "A designer dispatch execution request requires at least one admitted dispatch.",
        "#1325: dry-run designer execution JSON should report aggregate dispatch preflight errors");

    const auto unadmitted_execution_process = run_process_capture(
        studio_host_path,
        {
            "--designer-execute",
            "--selection-context", "visual_object",
            "--admit-editor-invocations", "true",
            "--admit-builder-invocations", "true",
            "--admit-toolbox-invocation", "true",
            "--admit-designer-execution", "false",
            "--editor-action-launch-command", COPPERFIN_TEST_SUCCESS_COMMAND,
            "--builder-launch-command", COPPERFIN_TEST_SUCCESS_COMMAND,
            "--toolbox-launch-command", COPPERFIN_TEST_SUCCESS_COMMAND,
            "--json"
        },
        temp_root);
    expect(unadmitted_execution_process.exit_code == 4,
        "#1325: designer execution JSON should require explicit aggregate execution admission");
    expect_contains(unadmitted_execution_process.stdout_text,
        "A designer dispatch execution request requires explicit execution admission.",
        "#1325: unadmitted designer execution JSON should report aggregate execution admission errors");
    expect_contains(unadmitted_execution_process.stdout_text, "\"executed\": false",
        "#1325: unadmitted designer execution JSON should not mark execution complete");

    const auto failed_builder_process = run_process_capture(
        studio_host_path,
        {
            "--designer-execute",
            "--selection-context", "visual_object",
            "--admit-editor-invocations", "true",
            "--admit-builder-invocations", "true",
            "--admit-toolbox-invocation", "true",
            "--admit-designer-execution", "true",
            "--editor-action-launch-command", COPPERFIN_TEST_SUCCESS_COMMAND,
            "--builder-launch-command", COPPERFIN_TEST_FAILURE_COMMAND,
            "--toolbox-launch-command", COPPERFIN_TEST_SUCCESS_COMMAND,
            "--json"
        },
        temp_root);
    expect(failed_builder_process.exit_code == 4,
        "#1325: designer execution JSON should fail when child executions fail");
    expect_contains(failed_builder_process.stdout_text,
        "Designer builder launch command returned a non-zero exit code.",
        "#1325: failed designer child execution JSON should expose child errors");
    expect_contains(failed_builder_process.stdout_text, "\"errorCount\": ",
        "#1325: failed designer execution JSON should expose execution error counts");
    expect_contains(failed_builder_process.stdout_text, "\"executionReadySelectionContexts\": []",
        "#1402: failed builder execution JSON should expose empty execution-ready selected contexts");
    expect_contains(failed_builder_process.stdout_text, "\"executionBlockedSelectionContexts\": [\"visual_object\"]",
        "#1402: failed builder execution JSON should summarize execution-blocked selected contexts");
    expect_contains(failed_builder_process.stdout_text,
        "\"executionBlockedErrors\": [\"Designer builder launch command returned a non-zero exit code.\"]",
        "#1402: failed builder execution JSON should summarize blocked selected-context errors");
    expect_contains(failed_builder_process.stdout_text, "\"executed\": false",
        "#1325: failed designer execution JSON should not mark aggregate execution complete");
    expect_contains(failed_builder_process.stdout_text, "\"builderId\": \"form-builder\"",
        "#1340: failed designer execution JSON should preserve planned builder identity");
    expect_contains(failed_builder_process.stdout_text, "\"editorActionExecutedCount\": 5",
        "#1344: failed builder execution JSON should preserve editor action executed counts");
    expect_contains(failed_builder_process.stdout_text, "\"builderExecutedCount\": 0",
        "#1344: failed builder execution JSON should expose zero builder executed counts");
    expect_contains(failed_builder_process.stdout_text, "\"toolboxExecutedCount\": 1",
        "#1344: failed builder execution JSON should preserve toolbox executed counts");
    expect_contains(failed_builder_process.stdout_text, "\"editorActionErrorCount\": 0",
        "#1343: failed builder execution JSON should preserve zero editor action errors");
    expect_contains(failed_builder_process.stdout_text, "\"builderErrorCount\": 3",
        "#1343: failed builder execution JSON should expose builder error counts");
    expect_contains(failed_builder_process.stdout_text, "\"toolboxErrorCount\": 0",
        "#1343: failed builder execution JSON should preserve zero toolbox errors");
    expect_contains(failed_builder_process.stdout_text,
        "\"failedBuilderIds\": [\"form-builder\", \"control-builder\", \"grid-builder\"]",
        "#1345: failed builder execution JSON should summarize failed builder ids");
    expect_contains(failed_builder_process.stdout_text,
        "\"failedBuilderCommandTokens\": [\"studio.builder.invoke\"",
        "#1353: failed builder execution JSON should summarize failed builder command tokens");
    expect_contains(failed_builder_process.stdout_text,
        "\"failedBuilderExecutedCommands\": [\"" + expected_json_shell_command(
            COPPERFIN_TEST_FAILURE_COMMAND,
            {"--command-token", "studio.builder.invoke"}),
        "#1354: failed builder execution JSON should summarize failed builder executed commands");
    expect_contains(failed_builder_process.stdout_text, "\"failedBuilderExitCodes\": [1, 1, 1]",
        "#1355: failed builder execution JSON should summarize failed builder exit codes");
    expect_contains(failed_builder_process.stdout_text,
        "\"failedBuilderErrors\": [\"Designer builder launch command returned a non-zero exit code.\"",
        "#1346: failed builder execution JSON should summarize failed builder errors");
    expect_contains(failed_builder_process.stdout_text,
        "\"launchCommand\": \"" COPPERFIN_TEST_FAILURE_COMMAND "\"",
        "#1348: failed aggregate builder execution JSON should expose child launch commands");
    expect_contains(failed_builder_process.stdout_text, "\"observedExitCode\": 1",
        "#1347: failed aggregate builder execution JSON should report normalized child exit codes");
    expect_contains(failed_builder_process.stdout_text, "\"failedEditorActionIds\": []",
        "#1345: failed builder execution JSON should preserve no failed editor actions");
    expect_contains(failed_builder_process.stdout_text, "\"failedEditorActionCommandTokens\": []",
        "#1353: failed builder execution JSON should preserve no failed editor action command tokens");
    expect_contains(failed_builder_process.stdout_text, "\"failedEditorActionExecutedCommands\": []",
        "#1354: failed builder execution JSON should preserve no failed editor action executed commands");
    expect_contains(failed_builder_process.stdout_text, "\"failedEditorActionExitCodes\": []",
        "#1355: failed builder execution JSON should preserve no failed editor action exit codes");
    expect_contains(failed_builder_process.stdout_text, "\"failedEditorActionErrors\": []",
        "#1346: failed builder execution JSON should preserve no failed editor action errors");
    expect_contains(failed_builder_process.stdout_text, "\"toolboxFailed\": false",
        "#1345: failed builder execution JSON should preserve successful toolbox summary state");
    expect_contains(failed_builder_process.stdout_text, "\"toolboxError\": \"\"",
        "#1346: failed builder execution JSON should preserve empty toolbox error summaries");
    expect_contains(failed_builder_process.stdout_text, "\"commandToken\": \"studio.builder.invoke\"",
        "#1340: failed designer execution JSON should preserve planned builder command token");
    expect_contains(failed_builder_process.stdout_text,
        "\"dispatchArguments\": [\"--command-token\", \"studio.builder.invoke\"",
        "#1340: failed designer execution JSON should preserve planned builder dispatch arguments");
    expect_contains(failed_builder_process.stdout_text,
        "\"executedCommand\": \"" + expected_json_shell_command(
            COPPERFIN_TEST_FAILURE_COMMAND,
            {"--command-token", "studio.builder.invoke"}),
        "#1340: failed designer execution JSON should preserve the failed builder command");

    const auto failed_editor_process = run_process_capture(
        studio_host_path,
        {
            "--designer-execute",
            "--selection-context", "visual_object",
            "--admit-editor-invocations", "true",
            "--admit-builder-invocations", "true",
            "--admit-toolbox-invocation", "true",
            "--admit-designer-execution", "true",
            "--editor-action-launch-command", COPPERFIN_TEST_FAILURE_COMMAND,
            "--builder-launch-command", COPPERFIN_TEST_SUCCESS_COMMAND,
            "--toolbox-launch-command", COPPERFIN_TEST_SUCCESS_COMMAND,
            "--json"
        },
        temp_root);
    expect(failed_editor_process.exit_code == 4,
        "#1341: designer execution JSON should fail when editor action executions fail");
    expect_contains(failed_editor_process.stdout_text,
        "Designer editor action launch command returned a non-zero exit code.",
        "#1341: failed editor action execution JSON should expose child errors");
    expect_contains(failed_editor_process.stdout_text, "\"actionId\": \"show-property-grid\"",
        "#1341: failed editor action execution JSON should preserve planned editor action identity");
    expect_contains(failed_editor_process.stdout_text, "\"editorActionExecutedCount\": 0",
        "#1344: failed editor action execution JSON should expose zero editor action executed counts");
    expect_contains(failed_editor_process.stdout_text, "\"builderExecutedCount\": 3",
        "#1344: failed editor action execution JSON should preserve builder executed counts");
    expect_contains(failed_editor_process.stdout_text, "\"toolboxExecutedCount\": 1",
        "#1344: failed editor action execution JSON should preserve toolbox executed counts");
    expect_contains(failed_editor_process.stdout_text, "\"editorActionErrorCount\": 5",
        "#1343: failed editor action execution JSON should expose editor action error counts");
    expect_contains(failed_editor_process.stdout_text, "\"builderErrorCount\": 0",
        "#1343: failed editor action execution JSON should preserve zero builder errors");
    expect_contains(failed_editor_process.stdout_text, "\"toolboxErrorCount\": 0",
        "#1343: failed editor action execution JSON should preserve zero toolbox errors");
    expect_contains(failed_editor_process.stdout_text,
        "\"failedEditorActionIds\": [\"show-property-grid\", \"edit-visual-method\"",
        "#1345: failed editor action execution JSON should summarize failed editor action ids");
    expect_contains(failed_editor_process.stdout_text,
        "\"failedEditorActionCommandTokens\": [\"studio.property_grid.show\", \"studio.method_editor.open\"",
        "#1353: failed editor action execution JSON should summarize failed editor action command tokens");
    expect_contains(failed_editor_process.stdout_text,
        "\"failedEditorActionExecutedCommands\": [\"" + expected_json_shell_command(
            COPPERFIN_TEST_FAILURE_COMMAND,
            {"--command-token", "studio.property_grid.show"}),
        "#1354: failed editor action execution JSON should summarize failed editor action executed commands");
    expect_contains(failed_editor_process.stdout_text, "\"failedEditorActionExitCodes\": [1, 1, 1, 1, 1]",
        "#1355: failed editor action execution JSON should summarize failed editor action exit codes");
    expect_contains(failed_editor_process.stdout_text,
        "\"failedEditorActionErrors\": [\"Designer editor action launch command returned a non-zero exit code.\"",
        "#1346: failed editor action execution JSON should summarize failed editor action errors");
    expect_contains(failed_editor_process.stdout_text,
        "\"launchCommand\": \"" COPPERFIN_TEST_FAILURE_COMMAND "\"",
        "#1348: failed aggregate editor action execution JSON should expose child launch commands");
    expect_contains(failed_editor_process.stdout_text, "\"observedExitCode\": 1",
        "#1347: failed aggregate editor action execution JSON should report normalized child exit codes");
    expect_contains(failed_editor_process.stdout_text, "\"failedBuilderIds\": []",
        "#1345: failed editor action execution JSON should preserve no failed builders");
    expect_contains(failed_editor_process.stdout_text, "\"failedBuilderCommandTokens\": []",
        "#1353: failed editor action execution JSON should preserve no failed builder command tokens");
    expect_contains(failed_editor_process.stdout_text, "\"failedBuilderExecutedCommands\": []",
        "#1354: failed editor action execution JSON should preserve no failed builder executed commands");
    expect_contains(failed_editor_process.stdout_text, "\"failedBuilderExitCodes\": []",
        "#1355: failed editor action execution JSON should preserve no failed builder exit codes");
    expect_contains(failed_editor_process.stdout_text, "\"failedBuilderErrors\": []",
        "#1346: failed editor action execution JSON should preserve no failed builder errors");
    expect_contains(failed_editor_process.stdout_text, "\"toolboxFailed\": false",
        "#1345: failed editor action execution JSON should preserve successful toolbox summary state");
    expect_contains(failed_editor_process.stdout_text, "\"toolboxError\": \"\"",
        "#1346: failed editor action execution JSON should preserve empty toolbox error summaries");
    expect_contains(failed_editor_process.stdout_text, "\"commandToken\": \"studio.property_grid.show\"",
        "#1341: failed editor action execution JSON should preserve planned editor action command token");
    expect_contains(failed_editor_process.stdout_text,
        "\"dispatchArguments\": [\"--command-token\", \"studio.property_grid.show\"",
        "#1341: failed editor action execution JSON should preserve planned editor action dispatch arguments");
    expect_contains(failed_editor_process.stdout_text,
        "\"executedCommand\": \"" + expected_json_shell_command(
            COPPERFIN_TEST_FAILURE_COMMAND,
            {"--command-token", "studio.property_grid.show"}),
        "#1341: failed editor action execution JSON should preserve the failed editor action command");

    const auto failed_toolbox_process = run_process_capture(
        studio_host_path,
        {
            "--designer-execute",
            "--selection-context", "visual_object",
            "--admit-editor-invocations", "true",
            "--admit-builder-invocations", "true",
            "--admit-toolbox-invocation", "true",
            "--admit-designer-execution", "true",
            "--editor-action-launch-command", COPPERFIN_TEST_SUCCESS_COMMAND,
            "--builder-launch-command", COPPERFIN_TEST_SUCCESS_COMMAND,
            "--toolbox-launch-command", COPPERFIN_TEST_FAILURE_COMMAND,
            "--json"
        },
        temp_root);
    expect(failed_toolbox_process.exit_code == 4,
        "#1341: designer execution JSON should fail when toolbox execution fails");
    expect_contains(failed_toolbox_process.stdout_text,
        "Designer toolbox launch command returned a non-zero exit code.",
        "#1341: failed toolbox execution JSON should expose child errors");
    expect_contains(failed_toolbox_process.stdout_text, "\"editorActionExecutionCount\": 5",
        "#1342: failed toolbox execution JSON should preserve editor action execution counts");
    expect_contains(failed_toolbox_process.stdout_text, "\"builderExecutionCount\": 3",
        "#1342: failed toolbox execution JSON should preserve builder execution counts");
    expect_contains(failed_toolbox_process.stdout_text, "\"toolboxExecutionCount\": 1",
        "#1342: failed toolbox execution JSON should preserve toolbox execution counts");
    expect_contains(failed_toolbox_process.stdout_text, "\"editorActionExecutedCount\": 5",
        "#1344: failed toolbox execution JSON should preserve editor action executed counts");
    expect_contains(failed_toolbox_process.stdout_text, "\"builderExecutedCount\": 3",
        "#1344: failed toolbox execution JSON should preserve builder executed counts");
    expect_contains(failed_toolbox_process.stdout_text, "\"toolboxExecutedCount\": 0",
        "#1344: failed toolbox execution JSON should expose zero toolbox executed counts");
    expect_contains(failed_toolbox_process.stdout_text, "\"editorActionErrorCount\": 0",
        "#1343: failed toolbox execution JSON should preserve zero editor action errors");
    expect_contains(failed_toolbox_process.stdout_text, "\"builderErrorCount\": 0",
        "#1343: failed toolbox execution JSON should preserve zero builder errors");
    expect_contains(failed_toolbox_process.stdout_text, "\"toolboxErrorCount\": 1",
        "#1343: failed toolbox execution JSON should expose toolbox error counts");
    expect_contains(failed_toolbox_process.stdout_text, "\"failedEditorActionIds\": []",
        "#1345: failed toolbox execution JSON should preserve no failed editor actions");
    expect_contains(failed_toolbox_process.stdout_text, "\"failedEditorActionCommandTokens\": []",
        "#1353: failed toolbox execution JSON should preserve no failed editor action command tokens");
    expect_contains(failed_toolbox_process.stdout_text, "\"failedEditorActionExecutedCommands\": []",
        "#1354: failed toolbox execution JSON should preserve no failed editor action executed commands");
    expect_contains(failed_toolbox_process.stdout_text, "\"failedEditorActionExitCodes\": []",
        "#1355: failed toolbox execution JSON should preserve no failed editor action exit codes");
    expect_contains(failed_toolbox_process.stdout_text, "\"failedEditorActionErrors\": []",
        "#1346: failed toolbox execution JSON should preserve no failed editor action errors");
    expect_contains(failed_toolbox_process.stdout_text, "\"failedBuilderIds\": []",
        "#1345: failed toolbox execution JSON should preserve no failed builders");
    expect_contains(failed_toolbox_process.stdout_text, "\"failedBuilderCommandTokens\": []",
        "#1353: failed toolbox execution JSON should preserve no failed builder command tokens");
    expect_contains(failed_toolbox_process.stdout_text, "\"failedBuilderExecutedCommands\": []",
        "#1354: failed toolbox execution JSON should preserve no failed builder executed commands");
    expect_contains(failed_toolbox_process.stdout_text, "\"failedBuilderExitCodes\": []",
        "#1355: failed toolbox execution JSON should preserve no failed builder exit codes");
    expect_contains(failed_toolbox_process.stdout_text, "\"failedBuilderErrors\": []",
        "#1346: failed toolbox execution JSON should preserve no failed builder errors");
    expect_contains(failed_toolbox_process.stdout_text, "\"toolboxFailed\": true",
        "#1345: failed toolbox execution JSON should summarize failed toolbox state");
    expect_contains(failed_toolbox_process.stdout_text,
        "\"toolboxError\": \"Designer toolbox launch command returned a non-zero exit code.\"",
        "#1346: failed toolbox execution JSON should summarize failed toolbox errors");
    expect_contains(failed_toolbox_process.stdout_text, "\"toolboxCommandToken\": \"studio.toolbox.palette.invoke\"",
        "#1353: failed toolbox execution JSON should summarize toolbox command tokens");
    expect_contains(failed_toolbox_process.stdout_text,
        "\"toolboxExecutedCommand\": \"" + expected_json_shell_command(
            COPPERFIN_TEST_FAILURE_COMMAND,
            {"--command-token", "studio.toolbox.palette.invoke"}),
        "#1354: failed toolbox execution JSON should summarize toolbox executed commands");
    expect_contains(failed_toolbox_process.stdout_text, "\"toolboxExitCode\": 1",
        "#1355: failed toolbox execution JSON should summarize toolbox exit codes");
    expect_contains(failed_toolbox_process.stdout_text,
        "\"launchCommand\": \"" COPPERFIN_TEST_FAILURE_COMMAND "\"",
        "#1348: failed aggregate toolbox execution JSON should expose child launch commands");
    expect_contains(failed_toolbox_process.stdout_text, "\"observedExitCode\": 1",
        "#1347: failed aggregate toolbox execution JSON should report normalized child exit codes");
    expect_contains(failed_toolbox_process.stdout_text, "\"toolboxContext\": \"form\"",
        "#1341: failed toolbox execution JSON should preserve planned toolbox context");
    expect_contains(failed_toolbox_process.stdout_text, "\"commandToken\": \"studio.toolbox.palette.invoke\"",
        "#1341: failed toolbox execution JSON should preserve planned toolbox command token");
    expect_contains(failed_toolbox_process.stdout_text,
        "\"dispatchArguments\": [\"--command-token\", \"studio.toolbox.palette.invoke\"",
        "#1341: failed toolbox execution JSON should preserve planned toolbox dispatch arguments");
    expect_contains(failed_toolbox_process.stdout_text,
        "\"executedCommand\": \"" + expected_json_shell_command(
            COPPERFIN_TEST_FAILURE_COMMAND,
            {"--command-token", "studio.toolbox.palette.invoke"}),
        "#1341: failed toolbox execution JSON should preserve the failed toolbox command");

    const auto unknown_context_process = run_process_capture(
        studio_host_path,
        {
            "--designer-execute",
            "--selection-context", "unknown",
            "--json"
        },
        temp_root);
    expect(unknown_context_process.exit_code == 2,
        "#1325: designer execution JSON should reject unknown selection contexts");
    expect_contains(unknown_context_process.stdout_text, "Unknown selection context token: unknown",
        "#1325: unknown designer execution context JSON should report parser errors");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_exposes_designer_dispatch_catalog(const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_designer_dispatch_catalog_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);
    ScopedDefaultLocaleCatalogEnvironment default_locale_environment;

    const auto catalog_process = run_process_capture(
        studio_host_path,
        {
            "--designer-dispatch-catalog",
            "--path", "forms/customer.scx",
            "--record", "1",
            "--object-name", "frmCustomer",
            "--unique-id", "form-guid",
            "--symbol", "Click",
            "--line", "12",
            "--column", "4",
            "--admit-editor-invocations", "true",
            "--admit-builder-invocations", "true",
            "--admit-toolbox-invocation", "true",
            "--json"
        },
        temp_root);
    expect(catalog_process.exit_code == 0,
        "#1240: designer dispatch catalog JSON should accept aggregate admission policies");
    expect_contains(catalog_process.stdout_text, "\"designerDispatchCatalog\": {",
        "#1240: designer dispatch catalog JSON should expose a catalog object");
    expect_contains(catalog_process.stdout_text, "\"contextCount\": 9",
        "#1240: designer dispatch catalog JSON should expose context counts");
    expect_contains(catalog_process.stdout_text, "\"dispatchOkSelectionContexts\": [\"visual_object\"",
        "#1357: designer dispatch catalog JSON should summarize dispatch-clean contexts");
    expect_contains(catalog_process.stdout_text, "\"dispatchBlockedSelectionContexts\": [\"menu_item\"",
        "#1357: designer dispatch catalog JSON should summarize dispatch-blocked contexts");
    expect_contains(catalog_process.stdout_text,
        "\"dispatchBlockedErrors\": [\"The selected Studio context does not expose a toolbox palette.\"",
        "#1357: designer dispatch catalog JSON should summarize blocked dispatch errors");
    expect_contains(catalog_process.stdout_text, "\"selectionContext\": \"visual_object\"",
        "#1240: designer dispatch catalog JSON should include visual-object contexts");
    expect_contains(catalog_process.stdout_text, "\"selectionContext\": \"report_expression\"",
        "#1240: designer dispatch catalog JSON should include report-expression contexts");
    expect_contains(catalog_process.stdout_text, "\"selectionContext\": \"menu_item\"",
        "#1240: designer dispatch catalog JSON should include menu-item contexts");
    expect_contains(catalog_process.stdout_text, "\"editorActionDispatchCount\": ",
        "#1240: designer dispatch catalog JSON should expose editor dispatch counts");
    expect_contains(catalog_process.stdout_text, "\"builderDispatchCount\": ",
        "#1240: designer dispatch catalog JSON should expose builder dispatch counts");
    expect_contains(catalog_process.stdout_text, "\"toolboxDispatchCount\": ",
        "#1240: designer dispatch catalog JSON should expose toolbox dispatch counts");
    expect_contains(catalog_process.stdout_text, "\"dispatchCount\": ",
        "#1240: designer dispatch catalog JSON should expose aggregate dispatch counts");
    expect_contains(catalog_process.stdout_text, "\"errorCount\": ",
        "#1240: designer dispatch catalog JSON should expose aggregate error counts");
    expect_contains(catalog_process.stdout_text, "\"dryRun\": false",
        "#1240: designer dispatch catalog JSON should expose non-dry-run admitted contexts");
    expect_contains(catalog_process.stdout_text, "\"mutatesAsset\": false",
        "#1240: designer dispatch catalog JSON should remain non-mutating");
    expect_contains(catalog_process.stdout_text, "\"dispatchOk\": true",
        "#1240: designer dispatch catalog JSON should expose nested aggregate dispatch status");
    expect_contains(catalog_process.stdout_text, "\"editorActionIds\": [\"edit-visual-method\"",
        "#1240: designer dispatch catalog JSON should expose admitted visual editor dispatch ids");
    expect_contains(catalog_process.stdout_text, "\"edit-report-expression\"",
        "#1240: designer dispatch catalog JSON should expose admitted report editor dispatch ids");
    expect_contains(catalog_process.stdout_text, "\"editorActionDispatches\": [",
        "#1336: designer dispatch catalog JSON should expose nested editor dispatch entries");
    expect_contains(catalog_process.stdout_text, "\"commandToken\": \"studio.method_editor.open\"",
        "#1336: designer dispatch catalog JSON should expose editor dispatch command tokens");
    expect_contains(catalog_process.stdout_text, "\"dispatchArguments\": [\"--command-token\", \"studio.method_editor.open\"",
        "#1336: designer dispatch catalog JSON should expose editor dispatch arguments");
    expect_contains(catalog_process.stdout_text, "\"builderDispatches\": [",
        "#1336: designer dispatch catalog JSON should expose nested builder dispatch entries");
    expect_contains(catalog_process.stdout_text, "\"commandToken\": \"studio.builder.invoke\"",
        "#1336: designer dispatch catalog JSON should expose builder dispatch command tokens");
    expect_contains(catalog_process.stdout_text, "\"dispatchArguments\": [\"--command-token\", \"studio.builder.invoke\"",
        "#1336: designer dispatch catalog JSON should expose builder dispatch arguments");
    expect_contains(catalog_process.stdout_text, "\"toolboxDispatchOk\": false",
        "#1240: designer dispatch catalog JSON should expose unsupported toolbox dispatch status");
    expect_contains(catalog_process.stdout_text, "\"toolboxCommandToken\": \"studio.toolbox.palette.invoke\"",
        "#1336: designer dispatch catalog JSON should expose toolbox dispatch command tokens");
    expect_contains(catalog_process.stdout_text,
        "\"toolboxDispatchArguments\": [\"--command-token\", \"studio.toolbox.palette.invoke\"",
        "#1336: designer dispatch catalog JSON should expose toolbox dispatch arguments");
    expect_contains(catalog_process.stdout_text,
        "\"toolboxError\": \"The selected Studio context does not expose a toolbox palette.\"",
        "#1240: designer dispatch catalog JSON should preserve unsupported toolbox reasons");

    const auto unadmitted_process = run_process_capture(
        studio_host_path,
        {
            "--designer-dispatch-catalog",
            "--json"
        },
        temp_root);
    expect(unadmitted_process.exit_code == 0,
        "#1357: designer dispatch catalog JSON should summarize unadmitted dispatch-blocked contexts");
    expect_contains(unadmitted_process.stdout_text, "\"dispatchOkSelectionContexts\": []",
        "#1357: unadmitted designer dispatch catalog JSON should summarize no dispatch-clean contexts");
    expect_contains(unadmitted_process.stdout_text, "\"dispatchBlockedSelectionContexts\": [\"visual_object\"",
        "#1357: unadmitted designer dispatch catalog JSON should summarize dispatch-blocked contexts");
    expect_contains(unadmitted_process.stdout_text,
        "\"dispatchBlockedErrors\": [\"An editor action dispatch request requires an admitted non-dry-run invocation.\"",
        "#1357: unadmitted designer dispatch catalog JSON should summarize child dispatch errors");

    const auto invalid_boolean_process = run_process_capture(
        studio_host_path,
        {
            "--designer-dispatch-catalog",
            "--admit-toolbox-invocation", "maybe",
            "--json"
        },
        temp_root);
    expect(invalid_boolean_process.exit_code == 2,
        "#1240: designer dispatch catalog JSON should reject invalid toolbox admission booleans");
    expect_contains(invalid_boolean_process.stdout_text, "\"designerDispatchCatalog\": null",
        "#1240: invalid designer dispatch catalog boolean JSON should not expose catalog objects");
    expect_contains(invalid_boolean_process.stdout_text,
        "The --admit-toolbox-invocation value must be true or false.",
        "#1240: invalid designer dispatch catalog boolean JSON should report parser errors");

    const auto invalid_line_process = run_process_capture(
        studio_host_path,
        {
            "--designer-dispatch-catalog",
            "--line", "-1",
            "--json"
        },
        temp_root);
    expect(invalid_line_process.exit_code == 2,
        "#1240: designer dispatch catalog JSON should reject invalid line values");
    expect_contains(invalid_line_process.stdout_text, "The --line value must be a non-negative integer.",
        "#1240: invalid designer dispatch catalog line JSON should report parser errors");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_exposes_designer_dispatch_execution_catalog(const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_designer_dispatch_execution_catalog_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);
    ScopedDefaultLocaleCatalogEnvironment default_locale_environment;

    const auto catalog_process = run_process_capture(
        studio_host_path,
        {
            "--designer-dispatch-execution-catalog",
            "--path", "forms/customer.scx",
            "--record", "1",
            "--object-name", "frmCustomer",
            "--unique-id", "form-guid",
            "--symbol", "Click",
            "--line", "12",
            "--column", "4",
            "--admit-editor-invocations", "true",
            "--admit-builder-invocations", "true",
            "--admit-toolbox-invocation", "true",
            "--admit-designer-execution", "true",
            "--json"
        },
        temp_root);
    expect(catalog_process.exit_code == 0,
        "#1333: designer dispatch execution catalog JSON should accept admitted aggregate policies");
    expect_contains(catalog_process.stdout_text, "\"designerDispatchExecutionCatalog\": {",
        "#1333: designer dispatch execution catalog JSON should expose a catalog object");
    expect_contains(catalog_process.stdout_text, "\"contextCount\": 9",
        "#1333: designer dispatch execution catalog JSON should expose context counts");
    expect_contains(catalog_process.stdout_text, "\"executionReadyCount\": ",
        "#1333: designer dispatch execution catalog JSON should expose readiness counts");
    expect_contains(catalog_process.stdout_text, "\"errorCount\": ",
        "#1333: designer dispatch execution catalog JSON should expose error counts");
    expect_contains(catalog_process.stdout_text, "\"readySelectionContexts\": [\"visual_object\"",
        "#1356: designer dispatch execution catalog JSON should summarize ready contexts");
    expect_contains(catalog_process.stdout_text, "\"blockedSelectionContexts\": [\"menu_item\"",
        "#1356: designer dispatch execution catalog JSON should summarize blocked contexts");
    expect_contains(catalog_process.stdout_text,
        "\"blockedExecutionErrors\": [\"A designer dispatch execution catalog entry requires an error-free dispatch plan.\"",
        "#1356: designer dispatch execution catalog JSON should summarize blocked context errors");
    expect_contains(catalog_process.stdout_text, "\"dryRun\": false",
        "#1333: admitted designer dispatch execution catalog JSON should expose non-dry-run state");
    expect_contains(catalog_process.stdout_text, "\"mutatesAsset\": false",
        "#1333: designer dispatch execution catalog JSON should remain non-mutating");
    expect_contains(catalog_process.stdout_text, "\"selectionContext\": \"visual_object\"",
        "#1333: designer dispatch execution catalog JSON should include visual-object contexts");
    expect_contains(catalog_process.stdout_text, "\"dispatchErrorCount\": 0",
        "#1333: visual-object execution catalog JSON should expose clean dispatch plans");
    expect_contains(catalog_process.stdout_text, "\"executionAdmitted\": true",
        "#1333: designer dispatch execution catalog JSON should expose execution admission");
    expect_contains(catalog_process.stdout_text, "\"executionReady\": true",
        "#1333: designer dispatch execution catalog JSON should expose ready contexts");
    expect_contains(catalog_process.stdout_text, "\"executionError\": \"\"",
        "#1333: admitted ready designer dispatch execution catalog JSON should expose empty execution errors");
    expect_contains(catalog_process.stdout_text, "\"editorActionIds\": [\"edit-visual-method\"",
        "#1333: designer dispatch execution catalog JSON should preserve editor action dispatch ids");
    expect_contains(catalog_process.stdout_text, "\"editorActionDispatches\": [",
        "#1334: designer dispatch execution catalog JSON should expose nested editor dispatch entries");
    expect_contains(catalog_process.stdout_text, "\"commandToken\": \"studio.method_editor.open\"",
        "#1334: designer dispatch execution catalog JSON should expose editor dispatch command tokens");
    expect_contains(catalog_process.stdout_text, "\"dispatchArguments\": [\"--command-token\", \"studio.method_editor.open\"",
        "#1334: designer dispatch execution catalog JSON should expose editor dispatch arguments");
    expect_contains(catalog_process.stdout_text, "\"builderIds\": [\"form-builder\"",
        "#1333: designer dispatch execution catalog JSON should preserve builder dispatch ids");
    expect_contains(catalog_process.stdout_text, "\"builderDispatches\": [",
        "#1334: designer dispatch execution catalog JSON should expose nested builder dispatch entries");
    expect_contains(catalog_process.stdout_text, "\"commandToken\": \"studio.builder.invoke\"",
        "#1334: designer dispatch execution catalog JSON should expose builder dispatch command tokens");
    expect_contains(catalog_process.stdout_text, "\"dispatchArguments\": [\"--command-token\", \"studio.builder.invoke\"",
        "#1334: designer dispatch execution catalog JSON should expose builder dispatch arguments");
    expect_contains(catalog_process.stdout_text, "\"toolboxDispatchOk\": true",
        "#1333: designer dispatch execution catalog JSON should expose toolbox dispatch status");
    expect_contains(catalog_process.stdout_text, "\"toolboxCommandToken\": \"studio.toolbox.palette.invoke\"",
        "#1334: designer dispatch execution catalog JSON should expose toolbox dispatch command tokens");
    expect_contains(catalog_process.stdout_text,
        "\"toolboxDispatchArguments\": [\"--command-token\", \"studio.toolbox.palette.invoke\"",
        "#1334: designer dispatch execution catalog JSON should expose toolbox dispatch arguments");
    expect_contains(catalog_process.stdout_text, "\"selectionContext\": \"menu_item\"",
        "#1333: designer dispatch execution catalog JSON should include menu contexts");
    expect_contains(catalog_process.stdout_text,
        "A designer dispatch execution catalog entry requires an error-free dispatch plan.",
        "#1333: designer dispatch execution catalog JSON should propagate dispatch-plan errors");

    const auto unadmitted_process = run_process_capture(
        studio_host_path,
        {
            "--designer-dispatch-execution-catalog",
            "--admit-editor-invocations", "true",
            "--admit-builder-invocations", "true",
            "--admit-toolbox-invocation", "true",
            "--admit-designer-execution", "false",
            "--json"
        },
        temp_root);
    expect(unadmitted_process.exit_code == 0,
        "#1333: designer dispatch execution catalog JSON should report unadmitted execution as catalog errors");
    expect_contains(unadmitted_process.stdout_text, "\"executionReadyCount\": 0",
        "#1333: unadmitted designer dispatch execution catalog JSON should expose zero readiness");
    expect_contains(unadmitted_process.stdout_text, "\"readySelectionContexts\": []",
        "#1356: unadmitted designer dispatch execution catalog JSON should summarize no ready contexts");
    expect_contains(unadmitted_process.stdout_text, "\"blockedSelectionContexts\": [\"visual_object\"",
        "#1356: unadmitted designer dispatch execution catalog JSON should summarize blocked contexts");
    expect_contains(unadmitted_process.stdout_text,
        "\"blockedExecutionErrors\": [\"A designer dispatch execution catalog entry requires explicit execution admission.\"",
        "#1356: unadmitted designer dispatch execution catalog JSON should summarize blocked errors");
    expect_contains(unadmitted_process.stdout_text, "\"executionAdmitted\": false",
        "#1333: unadmitted designer dispatch execution catalog JSON should expose admission false");
    expect_contains(unadmitted_process.stdout_text,
        "A designer dispatch execution catalog entry requires explicit execution admission.",
        "#1333: unadmitted designer dispatch execution catalog JSON should expose execution errors");

    const auto dry_run_process = run_process_capture(
        studio_host_path,
        {
            "--designer-dispatch-execution-catalog",
            "--admit-designer-execution", "true",
            "--json"
        },
        temp_root);
    expect(dry_run_process.exit_code == 0,
        "#1333: designer dispatch execution catalog JSON should report dry-run aggregate failures");
    expect_contains(dry_run_process.stdout_text, "\"executionReadyCount\": 0",
        "#1333: dry-run designer dispatch execution catalog JSON should expose zero readiness");
    expect_contains(dry_run_process.stdout_text,
        "A designer dispatch execution catalog entry requires at least one admitted dispatch.",
        "#1333: dry-run designer dispatch execution catalog JSON should expose aggregate preflight errors");

    const auto invalid_execution_bool_process = run_process_capture(
        studio_host_path,
        {
            "--designer-dispatch-execution-catalog",
            "--admit-designer-execution", "maybe",
            "--json"
        },
        temp_root);
    expect(invalid_execution_bool_process.exit_code == 2,
        "#1333: designer dispatch execution catalog JSON should reject invalid execution booleans");
    expect_contains(invalid_execution_bool_process.stdout_text,
        "The --admit-designer-execution value must be true or false.",
        "#1333: invalid designer execution catalog admission JSON should report parser errors");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_exposes_designer_launch_surface_catalog(const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_designer_launch_surface_catalog_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);
    ScopedDefaultLocaleCatalogEnvironment default_locale_environment;

    const auto catalog_process = run_process_capture(
        studio_host_path,
        {
            "--designer-launch-surface-catalog",
            "--path", "forms/customer.scx",
            "--record", "1",
            "--object-name", "frmCustomer",
            "--unique-id", "form-guid",
            "--symbol", "Click",
            "--line", "12",
            "--column", "4",
            "--json"
        },
        temp_root);
    expect(catalog_process.exit_code == 0,
        "#1214: designer launch-surface catalog JSON should exit successfully");
    expect_contains(catalog_process.stdout_text, "\"designerLaunchSurfaceCatalog\": {",
        "#1214: designer launch-surface catalog JSON should expose a catalog object");
    expect_contains(catalog_process.stdout_text, "\"contextCount\": 9",
        "#1214: designer launch-surface catalog JSON should expose all Studio contexts");
    expect_contains(catalog_process.stdout_text, "\"launchReadySelectionContexts\": [\"visual_object\"",
        "#1359: designer launch-surface catalog JSON should summarize launch-ready contexts");
    expect_contains(catalog_process.stdout_text, "\"launchBlockedSelectionContexts\": [\"menu_item\"",
        "#1359: designer launch-surface catalog JSON should summarize launch-blocked contexts");
    expect_contains(catalog_process.stdout_text,
        "\"launchBlockedErrors\": [\"The selected Studio context does not expose a toolbox palette.\"",
        "#1359: designer launch-surface catalog JSON should summarize blocked launch errors");
    expect_contains(catalog_process.stdout_text, "\"selectionContext\": \"visual_object\"",
        "#1214: designer launch-surface catalog JSON should include visual-object contexts");
    expect_contains(catalog_process.stdout_text, "\"selectionContext\": \"menu_item\"",
        "#1214: designer launch-surface catalog JSON should include menu contexts");
    expect_contains(catalog_process.stdout_text, "\"selectionContext\": \"project_item\"",
        "#1214: designer launch-surface catalog JSON should include project contexts");
    expect_contains(catalog_process.stdout_text, "\"selectionContext\": \"data_environment\"",
        "#1214: designer launch-surface catalog JSON should include data-environment contexts");
    expect_contains(catalog_process.stdout_text, "\"assetPath\": \"forms/customer.scx\"",
        "#1214: designer launch-surface catalog JSON should preserve asset paths in nested plans");
    expect_contains(catalog_process.stdout_text, "\"objectName\": \"frmCustomer\"",
        "#1214: designer launch-surface catalog JSON should preserve object names in nested plans");
    expect_contains(catalog_process.stdout_text, "\"symbol\": \"Click\"",
        "#1214: designer launch-surface catalog JSON should preserve editor symbols in nested plans");
    expect_contains(catalog_process.stdout_text, "\"editorActionIds\": [\"show-property-grid\"",
        "#1214: designer launch-surface catalog JSON should expose nested editor action ids");
    expect_contains(catalog_process.stdout_text, "\"editorActionIds\": [\"show-property-grid\", \"edit-menu-command\", \"open-builder\"]",
        "#1413: designer launch-surface catalog JSON should expose menu command editor action ids");
    expect_contains(catalog_process.stdout_text, "\"builderIds\": [\"form-builder\"",
        "#1214: designer launch-surface catalog JSON should expose nested builder ids");
    expect_contains(catalog_process.stdout_text, "\"builderIds\": [\"menu-designer\"]",
        "#1214: designer launch-surface catalog JSON should expose menu builder ids");
    expect_contains(catalog_process.stdout_text, "\"builderIds\": [\"application-wizard\"]",
        "#1214: designer launch-surface catalog JSON should expose project wizard ids");
    expect_contains(catalog_process.stdout_text, "\"builderIds\": [\"data-environment-builder\"]",
        "#1214: designer launch-surface catalog JSON should expose data-environment builder ids");
    expect_contains(catalog_process.stdout_text, "\"toolboxContext\": \"form\"",
        "#1214: designer launch-surface catalog JSON should expose visual toolbox context metadata");
    expect_contains(catalog_process.stdout_text,
        "\"toolboxError\": \"The selected Studio context does not expose a toolbox palette.\"",
        "#1214: designer launch-surface catalog JSON should expose unsupported toolbox reasons");

    const auto invalid_record_process = run_process_capture(
        studio_host_path,
        {
            "--designer-launch-surface-catalog",
            "--record", "-1",
            "--json"
        },
        temp_root);
    expect(invalid_record_process.exit_code == 2,
        "#1214: designer launch-surface catalog JSON should reject invalid records");
    expect_contains(invalid_record_process.stdout_text, "\"designerLaunchSurfaceCatalog\": null",
        "#1214: invalid catalog records should not expose a catalog object");
    expect_contains(invalid_record_process.stdout_text, "The --record value must be a non-negative integer.",
        "#1214: invalid catalog record JSON should report parser errors");

    const auto invalid_column_process = run_process_capture(
        studio_host_path,
        {
            "--designer-launch-surface-catalog",
            "--column", "-1",
            "--json"
        },
        temp_root);
    expect(invalid_column_process.exit_code == 2,
        "#1214: designer launch-surface catalog JSON should reject invalid columns");
    expect_contains(invalid_column_process.stdout_text, "The --column value must be a non-negative integer.",
        "#1214: invalid catalog column JSON should report parser errors");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_exposes_designer_invocation_admission_catalog(const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_designer_invocation_admission_catalog_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);
    ScopedDefaultLocaleCatalogEnvironment default_locale_environment;

    const auto catalog_process = run_process_capture(
        studio_host_path,
        {
            "--designer-invocation-admission-catalog",
            "--path", "forms/customer.scx",
            "--record", "1",
            "--object-name", "frmCustomer",
            "--unique-id", "form-guid",
            "--symbol", "Click",
            "--line", "12",
            "--column", "4",
            "--admit-editor-invocations", "true",
            "--admit-builder-invocations", "false",
            "--admit-toolbox-invocation", "true",
            "--json"
        },
        temp_root);
    expect(catalog_process.exit_code == 0,
        "#1224: designer invocation-admission catalog JSON should exit successfully");
    expect_contains(catalog_process.stdout_text, "\"designerInvocationAdmissionCatalog\": {",
        "#1224: designer invocation-admission catalog JSON should expose a catalog object");
    expect_contains(catalog_process.stdout_text, "\"contextCount\": 9",
        "#1224: designer invocation-admission catalog JSON should expose all Studio contexts");
    expect_contains(catalog_process.stdout_text, "\"admissionOkSelectionContexts\": [\"visual_object\"",
        "#1358: designer invocation-admission catalog JSON should summarize admission-clean contexts");
    expect_contains(catalog_process.stdout_text, "\"admissionBlockedSelectionContexts\": [\"menu_item\"",
        "#1358: designer invocation-admission catalog JSON should summarize admission-blocked contexts");
    expect_contains(catalog_process.stdout_text,
        "\"admissionBlockedErrors\": [\"The selected Studio context does not expose a toolbox palette.\"",
        "#1358: designer invocation-admission catalog JSON should summarize blocked admission errors");
    expect_contains(catalog_process.stdout_text, "\"selectionContext\": \"visual_object\"",
        "#1224: designer invocation-admission catalog JSON should include visual-object contexts");
    expect_contains(catalog_process.stdout_text, "\"selectionContext\": \"report_expression\"",
        "#1224: designer invocation-admission catalog JSON should include report contexts");
    expect_contains(catalog_process.stdout_text, "\"selectionContext\": \"menu_item\"",
        "#1224: designer invocation-admission catalog JSON should include menu contexts");
    expect_contains(catalog_process.stdout_text, "\"selectionContext\": \"data_environment\"",
        "#1224: designer invocation-admission catalog JSON should include data-environment contexts");
    expect_contains(catalog_process.stdout_text, "\"editorActionIds\": [\"show-property-grid\"",
        "#1224: designer invocation-admission catalog JSON should expose nested editor action ids");
    expect_contains(catalog_process.stdout_text, "\"editorActionIds\": [\"show-property-grid\", \"edit-menu-command\", \"open-builder\"]",
        "#1413: designer invocation-admission catalog JSON should expose menu command editor action ids");
    expect_contains(catalog_process.stdout_text, "\"builderIds\": [\"form-builder\"",
        "#1224: designer invocation-admission catalog JSON should expose visual builder ids");
    expect_contains(catalog_process.stdout_text, "\"builderIds\": [\"menu-designer\"]",
        "#1224: designer invocation-admission catalog JSON should expose menu builder ids");
    expect_contains(catalog_process.stdout_text, "\"builderIds\": [\"data-environment-builder\"]",
        "#1224: designer invocation-admission catalog JSON should expose data-environment builder ids");
    expect_contains(catalog_process.stdout_text, "\"editorInvocationsAdmitted\": true",
        "#1224: designer invocation-admission catalog JSON should expose editor admission policy");
    expect_contains(catalog_process.stdout_text, "\"builderInvocationsAdmitted\": false",
        "#1224: designer invocation-admission catalog JSON should expose builder admission policy");
    expect_contains(catalog_process.stdout_text, "\"toolboxInvocationAdmitted\": true",
        "#1224: designer invocation-admission catalog JSON should expose toolbox admission policy");
    expect_contains(catalog_process.stdout_text,
        "\"toolboxError\": \"The selected Studio context does not expose a toolbox palette.\"",
        "#1224: designer invocation-admission catalog JSON should expose unsupported toolbox reasons");
    expect_contains(catalog_process.stdout_text, "\"mutatesAsset\": false",
        "#1224: designer invocation-admission catalog JSON should remain non-mutating");

    const auto unadmitted_process = run_process_capture(
        studio_host_path,
        {
            "--designer-invocation-admission-catalog",
            "--json"
        },
        temp_root);
    expect(unadmitted_process.exit_code == 0,
        "#1358: designer invocation-admission catalog JSON should summarize dry-run admission catalogs");
    expect_contains(unadmitted_process.stdout_text, "\"admissionOkSelectionContexts\": [\"visual_object\"",
        "#1358: dry-run designer invocation-admission catalog JSON should preserve admission-clean contexts");
    expect_contains(unadmitted_process.stdout_text, "\"admissionBlockedSelectionContexts\": [\"menu_item\"",
        "#1358: dry-run designer invocation-admission catalog JSON should summarize admission-blocked contexts");
    expect_contains(unadmitted_process.stdout_text,
        "\"admissionBlockedErrors\": [\"The selected Studio context does not expose a toolbox palette.\"",
        "#1358: dry-run designer invocation-admission catalog JSON should summarize blocked admission errors");

    const auto invalid_record_process = run_process_capture(
        studio_host_path,
        {
            "--designer-invocation-admission-catalog",
            "--record", "-1",
            "--json"
        },
        temp_root);
    expect(invalid_record_process.exit_code == 2,
        "#1224: designer invocation-admission catalog JSON should reject invalid records");
    expect_contains(invalid_record_process.stdout_text, "\"designerInvocationAdmissionCatalog\": null",
        "#1224: invalid catalog records should not expose a catalog object");
    expect_contains(invalid_record_process.stdout_text, "The --record value must be a non-negative integer.",
        "#1224: invalid catalog record JSON should report parser errors");

    const auto invalid_boolean_process = run_process_capture(
        studio_host_path,
        {
            "--designer-invocation-admission-catalog",
            "--admit-toolbox-invocation", "maybe",
            "--json"
        },
        temp_root);
    expect(invalid_boolean_process.exit_code == 2,
        "#1224: designer invocation-admission catalog JSON should reject invalid booleans");
    expect_contains(invalid_boolean_process.stdout_text,
        "The --admit-toolbox-invocation value must be true or false.",
        "#1224: invalid catalog boolean JSON should report parser errors");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

}  // namespace cf_test_studio_host_json
