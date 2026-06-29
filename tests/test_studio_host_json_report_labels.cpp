#include "test_studio_host_json_support.h"

namespace cf_test_studio_host_json {
void test_studio_host_json_exposes_selected_group_header_label_sections_by_record_selection(
    const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_selected_group_header_label_sections_record_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path label_path = temp_root / "selected_group_header_section_record.lbx";
    write_synthetic_report_table_for_stable_group_section_expression_json(label_path);

    const auto section_process = run_process_capture(
        studio_host_path,
        {"--path", label_path.string(), "--record", "1", "--json"},
        temp_root);

    if (section_process.exit_code != 0) {
        std::cerr << "studio host record-selected group-header label section stdout:\n"
                  << section_process.stdout_text << "\n";
        std::cerr << "studio host record-selected group-header label section stderr:\n"
                  << section_process.stderr_text << "\n";
        std::cerr << "fixture root: " << temp_root << "\n";
    }

    expect(section_process.exit_code == 0,
           "#1981: record-selected group-header label section JSON should exit successfully");
    expect_contains(section_process.stdout_text,
                    "\"documentTitle\": \"selected_group_header_section_record.lbx\"",
                    "#1981: record-selected group-header label section JSON should preserve document titles");
    expect_contains(section_process.stdout_text, "\"isLabel\": true",
                    "#1981: record-selected group-header label section JSON should retain label identity");
    expect_contains(section_process.stdout_text, "\"selectedReportSectionAvailable\": true",
                    "#1981: record-selected group-header label sections should advertise selected-section availability");
    expect_contains(section_process.stdout_text, "\"selectedReportSelectionAvailable\": true",
                    "#1981: record-selected group-header label sections should advertise report-selection availability");
    expect_contains(section_process.stdout_text, "\"selectedReportSelectionKind\": \"section\"",
                    "#1981: record-selected group-header label sections should expose section selection kind");
    expect_contains(section_process.stdout_text, "\"previewBoundsAvailable\": true",
                    "#1981: record-selected group-header label section JSON should expose live preview availability");
    expect_contains(section_process.stdout_text, "\"previewBoundsLeft\": 0",
                    "#1981: record-selected group-header label section JSON should preserve live preview left bounds");
    expect_contains(section_process.stdout_text, "\"previewBoundsTop\": 0",
                    "#1981: record-selected group-header label section JSON should preserve live preview top bounds");
    expect_contains(section_process.stdout_text, "\"previewBoundsRight\": 0",
                    "#1981: record-selected group-header label section JSON should preserve live preview right bounds");
    expect_contains(section_process.stdout_text, "\"previewBoundsBottom\": 4100",
                    "#1981: record-selected group-header label section JSON should preserve live preview bottom bounds");
    expect_contains(section_process.stdout_text, "\"previewBoundsWidth\": 0",
                    "#1981: record-selected group-header label section JSON should preserve live preview widths");
    expect_contains(section_process.stdout_text, "\"previewBoundsHeight\": 4100",
                    "#1981: record-selected group-header label section JSON should preserve live preview heights");
    expect_contains(section_process.stdout_text, "\"deletedPreviewBoundsAvailable\": false",
                    "#1981: record-selected group-header label section JSON should not fabricate deleted preview availability");
    expect_contains(section_process.stdout_text, "\"selectedReportObjectAvailable\": false",
                    "#1981: record-selected group-header label sections should not advertise selected-object availability");
    expect_contains(section_process.stdout_text, "\"selectedReportObject\": null",
                    "#1981: record-selected group-header label sections should serialize null selected objects");
    expect_contains(section_process.stdout_text, "\"selectedReportObjectSectionAvailable\": false",
                    "#1981: record-selected group-header label sections should not advertise selected object-section availability");
    expect_contains(section_process.stdout_text, "\"selectedReportObjectSection\": null",
                    "#1981: record-selected group-header label sections should serialize null selected object sections");
    expect_contains(section_process.stdout_text, "\"selectedReportSettingsAvailable\": false",
                    "#1981: record-selected group-header label sections should not advertise selected-settings availability");
    expect_contains(section_process.stdout_text, "\"selectedReportSettings\": null",
                    "#1981: record-selected group-header label sections should serialize null selected settings");
    expect_contains(section_process.stdout_text, "\"sectionCount\": 3",
                    "#1981: record-selected group-header label section JSON should preserve live section counts");
    expect_contains(section_process.stdout_text, "\"deletedSectionCount\": 0",
                    "#1981: record-selected group-header label section JSON should preserve deleted section counts");
    expect_contains_in_order(
        section_process.stdout_text,
        {
            "\"sections\": [",
            "\"bandKind\": \"group_header\"",
            "\"expression\": \"customer.country\"",
            "\"expressionFieldIndex\": 2",
            "\"expressionMemoBlockNumber\": 2",
            "\"recordIndex\": 1",
            "\"bandKind\": \"detail\"",
            "\"recordIndex\": 2",
            "\"bandKind\": \"group_footer\"",
            "\"expression\": \"customer.country\"",
            "\"recordIndex\": 3"
        },
        "#1981: record-selected group-header label section JSON should expose sibling section metadata");
    expect_contains_in_order(
        section_process.stdout_text,
        {
            "\"selectedReportSection\": {",
            "\"id\": \"group_header_1\"",
            "\"bandKind\": \"group_header\"",
            "\"expression\": \"customer.country\"",
            "\"expressionFieldIndex\": 2",
            "\"expressionMemoBlockNumber\": 2",
            "\"recordIndex\": 1",
            "\"deleted\": false",
            "\"sectionIndex\": 0",
            "\"sectionCount\": 3",
            "\"top\": 0",
            "\"height\": 600",
            "\"bottom\": 600"
        },
        "#1981: record-selected group-header label sections should expose selected expression metadata");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_exposes_selected_group_footer_label_sections_by_record_selection(
    const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_selected_group_footer_label_sections_record_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path label_path = temp_root / "selected_group_footer_section_record.lbx";
    write_synthetic_report_table_for_stable_group_section_expression_json(label_path);

    const auto section_process = run_process_capture(
        studio_host_path,
        {"--path", label_path.string(), "--record", "3", "--json"},
        temp_root);

    if (section_process.exit_code != 0) {
        std::cerr << "studio host record-selected group-footer label section stdout:\n"
                  << section_process.stdout_text << "\n";
        std::cerr << "studio host record-selected group-footer label section stderr:\n"
                  << section_process.stderr_text << "\n";
        std::cerr << "fixture root: " << temp_root << "\n";
    }

    expect(section_process.exit_code == 0,
           "#1983: record-selected group-footer label section JSON should exit successfully");
    expect_contains(section_process.stdout_text,
                    "\"documentTitle\": \"selected_group_footer_section_record.lbx\"",
                    "#1983: record-selected group-footer label section JSON should preserve document titles");
    expect_contains(section_process.stdout_text, "\"isLabel\": true",
                    "#1983: record-selected group-footer label section JSON should retain label identity");
    expect_contains(section_process.stdout_text, "\"selectedReportSectionAvailable\": true",
                    "#1983: record-selected group-footer label sections should advertise selected-section availability");
    expect_contains(section_process.stdout_text, "\"selectedReportSelectionAvailable\": true",
                    "#1983: record-selected group-footer label sections should advertise report-selection availability");
    expect_contains(section_process.stdout_text, "\"selectedReportSelectionKind\": \"section\"",
                    "#1983: record-selected group-footer label sections should expose section selection kind");
    expect_contains(section_process.stdout_text, "\"previewBoundsAvailable\": true",
                    "#1983: record-selected group-footer label section JSON should expose live preview availability");
    expect_contains(section_process.stdout_text, "\"previewBoundsLeft\": 0",
                    "#1983: record-selected group-footer label section JSON should preserve live preview left bounds");
    expect_contains(section_process.stdout_text, "\"previewBoundsTop\": 0",
                    "#1983: record-selected group-footer label section JSON should preserve live preview top bounds");
    expect_contains(section_process.stdout_text, "\"previewBoundsRight\": 0",
                    "#1983: record-selected group-footer label section JSON should preserve live preview right bounds");
    expect_contains(section_process.stdout_text, "\"previewBoundsBottom\": 4100",
                    "#1983: record-selected group-footer label section JSON should preserve live preview bottom bounds");
    expect_contains(section_process.stdout_text, "\"previewBoundsWidth\": 0",
                    "#1983: record-selected group-footer label section JSON should preserve live preview widths");
    expect_contains(section_process.stdout_text, "\"previewBoundsHeight\": 4100",
                    "#1983: record-selected group-footer label section JSON should preserve live preview heights");
    expect_contains(section_process.stdout_text, "\"deletedPreviewBoundsAvailable\": false",
                    "#1983: record-selected group-footer label section JSON should not fabricate deleted preview availability");
    expect_contains(section_process.stdout_text, "\"selectedReportObjectAvailable\": false",
                    "#1983: record-selected group-footer label sections should not advertise selected-object availability");
    expect_contains(section_process.stdout_text, "\"selectedReportObject\": null",
                    "#1983: record-selected group-footer label sections should serialize null selected objects");
    expect_contains(section_process.stdout_text, "\"selectedReportObjectSectionAvailable\": false",
                    "#1983: record-selected group-footer label sections should not advertise selected object-section availability");
    expect_contains(section_process.stdout_text, "\"selectedReportObjectSection\": null",
                    "#1983: record-selected group-footer label sections should serialize null selected object sections");
    expect_contains(section_process.stdout_text, "\"selectedReportSettingsAvailable\": false",
                    "#1983: record-selected group-footer label sections should not advertise selected-settings availability");
    expect_contains(section_process.stdout_text, "\"selectedReportSettings\": null",
                    "#1983: record-selected group-footer label sections should serialize null selected settings");
    expect_contains(section_process.stdout_text, "\"sectionCount\": 3",
                    "#1983: record-selected group-footer label section JSON should preserve live section counts");
    expect_contains(section_process.stdout_text, "\"deletedSectionCount\": 0",
                    "#1983: record-selected group-footer label section JSON should preserve deleted section counts");
    expect_contains_in_order(
        section_process.stdout_text,
        {
            "\"sections\": [",
            "\"bandKind\": \"group_header\"",
            "\"expression\": \"customer.country\"",
            "\"recordIndex\": 1",
            "\"bandKind\": \"detail\"",
            "\"recordIndex\": 2",
            "\"bandKind\": \"group_footer\"",
            "\"expression\": \"customer.country\"",
            "\"expressionFieldIndex\": 2",
            "\"expressionMemoBlockNumber\": 3",
            "\"recordIndex\": 3"
        },
        "#1983: record-selected group-footer label section JSON should expose sibling section metadata");
    expect_contains_in_order(
        section_process.stdout_text,
        {
            "\"selectedReportSection\": {",
            "\"id\": \"group_footer_3\"",
            "\"bandKind\": \"group_footer\"",
            "\"expression\": \"customer.country\"",
            "\"expressionFieldIndex\": 2",
            "\"expressionMemoBlockNumber\": 3",
            "\"recordIndex\": 3",
            "\"deleted\": false",
            "\"sectionIndex\": 2",
            "\"sectionCount\": 3",
            "\"top\": 3600",
            "\"height\": 500",
            "\"bottom\": 4100"
        },
        "#1983: record-selected group-footer label sections should expose selected expression metadata");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_exposes_selected_deleted_group_header_label_sections_by_record_selection(
    const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_selected_deleted_group_header_label_sections_record_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path label_path = temp_root / "selected_deleted_group_header_section_record.lbx";
    write_synthetic_report_table_for_stable_group_section_expression_json(label_path);
    const auto delete_result = copperfin::vfp::set_record_deleted_flag(label_path.string(), 1U, true);
    expect(delete_result.ok && dbf_record_deleted(label_path, 1U),
           "#1985: record-selected deleted group-header label fixture should mark the group-header section deleted");

    const auto section_process = run_process_capture(
        studio_host_path,
        {"--path", label_path.string(), "--record", "1", "--json"},
        temp_root);

    if (section_process.exit_code != 0) {
        std::cerr << "studio host record-selected deleted group-header label section stdout:\n"
                  << section_process.stdout_text << "\n";
        std::cerr << "studio host record-selected deleted group-header label section stderr:\n"
                  << section_process.stderr_text << "\n";
        std::cerr << "fixture root: " << temp_root << "\n";
    }

    expect(section_process.exit_code == 0,
           "#1985: record-selected deleted group-header label section JSON should exit successfully");
    expect_contains(section_process.stdout_text,
                    "\"documentTitle\": \"selected_deleted_group_header_section_record.lbx\"",
                    "#1985: record-selected deleted group-header label section JSON should preserve document titles");
    expect_contains(section_process.stdout_text, "\"isLabel\": true",
                    "#1985: record-selected deleted group-header label section JSON should retain label identity");
    expect_contains(section_process.stdout_text, "\"selectedReportSectionAvailable\": true",
                    "#1985: record-selected deleted group-header label sections should advertise selected-section availability");
    expect_contains(section_process.stdout_text, "\"selectedReportSelectionAvailable\": true",
                    "#1985: record-selected deleted group-header label sections should advertise report-selection availability");
    expect_contains(section_process.stdout_text, "\"selectedReportSelectionKind\": \"section\"",
                    "#1985: record-selected deleted group-header label sections should expose section selection kind");
    expect_contains(section_process.stdout_text, "\"previewBoundsAvailable\": true",
                    "#1985: record-selected deleted group-header label section JSON should expose live preview availability");
    expect_contains(section_process.stdout_text, "\"previewBoundsLeft\": 0",
                    "#1985: record-selected deleted group-header label section JSON should preserve live preview left bounds");
    expect_contains(section_process.stdout_text, "\"previewBoundsTop\": 600",
                    "#1985: record-selected deleted group-header label section JSON should refresh live preview top bounds");
    expect_contains(section_process.stdout_text, "\"previewBoundsRight\": 0",
                    "#1985: record-selected deleted group-header label section JSON should preserve live preview right bounds");
    expect_contains(section_process.stdout_text, "\"previewBoundsBottom\": 4100",
                    "#1985: record-selected deleted group-header label section JSON should preserve live preview bottom bounds");
    expect_contains(section_process.stdout_text, "\"previewBoundsWidth\": 0",
                    "#1985: record-selected deleted group-header label section JSON should preserve live preview widths");
    expect_contains(section_process.stdout_text, "\"previewBoundsHeight\": 3500",
                    "#1985: record-selected deleted group-header label section JSON should refresh live preview heights");
    expect_contains(section_process.stdout_text, "\"deletedPreviewBoundsAvailable\": true",
                    "#1985: record-selected deleted group-header label section JSON should expose deleted preview availability");
    expect_contains(section_process.stdout_text, "\"deletedPreviewBoundsLeft\": 0",
                    "#1985: record-selected deleted group-header label section JSON should preserve deleted preview left bounds");
    expect_contains(section_process.stdout_text, "\"deletedPreviewBoundsTop\": 0",
                    "#1985: record-selected deleted group-header label section JSON should preserve deleted preview top bounds");
    expect_contains(section_process.stdout_text, "\"deletedPreviewBoundsRight\": 0",
                    "#1985: record-selected deleted group-header label section JSON should preserve deleted preview right bounds");
    expect_contains(section_process.stdout_text, "\"deletedPreviewBoundsBottom\": 600",
                    "#1985: record-selected deleted group-header label section JSON should preserve deleted preview bottom bounds");
    expect_contains(section_process.stdout_text, "\"deletedPreviewBoundsWidth\": 0",
                    "#1985: record-selected deleted group-header label section JSON should preserve deleted preview widths");
    expect_contains(section_process.stdout_text, "\"deletedPreviewBoundsHeight\": 600",
                    "#1985: record-selected deleted group-header label section JSON should preserve deleted preview heights");
    expect_contains(section_process.stdout_text, "\"selectedReportObjectAvailable\": false",
                    "#1985: record-selected deleted group-header label sections should not advertise selected-object availability");
    expect_contains(section_process.stdout_text, "\"selectedReportObject\": null",
                    "#1985: record-selected deleted group-header label sections should serialize null selected objects");
    expect_contains(section_process.stdout_text, "\"selectedReportObjectSectionAvailable\": false",
                    "#1985: record-selected deleted group-header label sections should not advertise selected object-section availability");
    expect_contains(section_process.stdout_text, "\"selectedReportObjectSection\": null",
                    "#1985: record-selected deleted group-header label sections should serialize null selected object sections");
    expect_contains(section_process.stdout_text, "\"selectedReportSettingsAvailable\": false",
                    "#1985: record-selected deleted group-header label sections should not advertise selected-settings availability");
    expect_contains(section_process.stdout_text, "\"selectedReportSettings\": null",
                    "#1985: record-selected deleted group-header label sections should serialize null selected settings");
    expect_contains(section_process.stdout_text, "\"sectionCount\": 2",
                    "#1985: record-selected deleted group-header label section JSON should preserve live section counts");
    expect_contains(section_process.stdout_text, "\"deletedSectionCount\": 1",
                    "#1985: record-selected deleted group-header label section JSON should expose deleted section counts");
    expect_contains_in_order(
        section_process.stdout_text,
        {
            "\"deletedSections\": [",
            "\"id\": \"group_header_1\"",
            "\"bandKind\": \"group_header\"",
            "\"expression\": \"customer.country\"",
            "\"expressionFieldIndex\": 2",
            "\"expressionMemoBlockNumber\": 2",
            "\"recordIndex\": 1",
            "\"deleted\": true"
        },
        "#1985: record-selected deleted group-header label section JSON should expose deleted section metadata");
    expect_contains_in_order(
        section_process.stdout_text,
        {
            "\"selectedReportSection\": {",
            "\"id\": \"group_header_1\"",
            "\"bandKind\": \"group_header\"",
            "\"expression\": \"customer.country\"",
            "\"expressionFieldIndex\": 2",
            "\"expressionMemoBlockNumber\": 2",
            "\"recordIndex\": 1",
            "\"deleted\": true",
            "\"sectionIndex\": null",
            "\"sectionCount\": 0",
            "\"top\": 0",
            "\"height\": 600",
            "\"bottom\": 600"
        },
        "#1985: record-selected deleted group-header label sections should expose selected expression metadata");
    expect_contains_in_order(
        section_process.stdout_text,
        {
            "\"sections\": [",
            "\"bandKind\": \"detail\"",
            "\"recordIndex\": 2",
            "\"bandKind\": \"group_footer\"",
            "\"recordIndex\": 3"
        },
        "#1985: record-selected deleted group-header label section JSON should preserve live sibling metadata");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_exposes_selected_deleted_group_footer_label_sections_by_record_selection(
    const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_selected_deleted_group_footer_label_sections_record_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path label_path = temp_root / "selected_deleted_group_footer_section_record.lbx";
    write_synthetic_report_table_for_stable_group_section_expression_json(label_path);
    const auto delete_result = copperfin::vfp::set_record_deleted_flag(label_path.string(), 3U, true);
    expect(delete_result.ok && dbf_record_deleted(label_path, 3U),
           "#1987: record-selected deleted group-footer label fixture should mark the group-footer section deleted");

    const auto section_process = run_process_capture(
        studio_host_path,
        {"--path", label_path.string(), "--record", "3", "--json"},
        temp_root);

    if (section_process.exit_code != 0) {
        std::cerr << "studio host record-selected deleted group-footer label section stdout:\n"
                  << section_process.stdout_text << "\n";
        std::cerr << "studio host record-selected deleted group-footer label section stderr:\n"
                  << section_process.stderr_text << "\n";
        std::cerr << "fixture root: " << temp_root << "\n";
    }

    expect(section_process.exit_code == 0,
           "#1987: record-selected deleted group-footer label section JSON should exit successfully");
    expect_contains(section_process.stdout_text,
                    "\"documentTitle\": \"selected_deleted_group_footer_section_record.lbx\"",
                    "#1987: record-selected deleted group-footer label section JSON should preserve document titles");
    expect_contains(section_process.stdout_text, "\"isLabel\": true",
                    "#1987: record-selected deleted group-footer label section JSON should retain label identity");
    expect_contains(section_process.stdout_text, "\"selectedReportSectionAvailable\": true",
                    "#1987: record-selected deleted group-footer label sections should advertise selected-section availability");
    expect_contains(section_process.stdout_text, "\"selectedReportSelectionAvailable\": true",
                    "#1987: record-selected deleted group-footer label sections should advertise report-selection availability");
    expect_contains(section_process.stdout_text, "\"selectedReportSelectionKind\": \"section\"",
                    "#1987: record-selected deleted group-footer label sections should expose section selection kind");
    expect_contains(section_process.stdout_text, "\"previewBoundsAvailable\": true",
                    "#1987: record-selected deleted group-footer label section JSON should expose live preview availability");
    expect_contains(section_process.stdout_text, "\"previewBoundsLeft\": 0",
                    "#1987: record-selected deleted group-footer label section JSON should preserve live preview left bounds");
    expect_contains(section_process.stdout_text, "\"previewBoundsTop\": 0",
                    "#1987: record-selected deleted group-footer label section JSON should preserve live preview top bounds");
    expect_contains(section_process.stdout_text, "\"previewBoundsRight\": 0",
                    "#1987: record-selected deleted group-footer label section JSON should preserve live preview right bounds");
    expect_contains(section_process.stdout_text, "\"previewBoundsBottom\": 3600",
                    "#1987: record-selected deleted group-footer label section JSON should refresh live preview bottom bounds");
    expect_contains(section_process.stdout_text, "\"previewBoundsWidth\": 0",
                    "#1987: record-selected deleted group-footer label section JSON should preserve live preview widths");
    expect_contains(section_process.stdout_text, "\"previewBoundsHeight\": 3600",
                    "#1987: record-selected deleted group-footer label section JSON should refresh live preview heights");
    expect_contains(section_process.stdout_text, "\"deletedPreviewBoundsAvailable\": true",
                    "#1987: record-selected deleted group-footer label section JSON should expose deleted preview availability");
    expect_contains(section_process.stdout_text, "\"deletedPreviewBoundsLeft\": 0",
                    "#1987: record-selected deleted group-footer label section JSON should preserve deleted preview left bounds");
    expect_contains(section_process.stdout_text, "\"deletedPreviewBoundsTop\": 3600",
                    "#1987: record-selected deleted group-footer label section JSON should preserve deleted preview top bounds");
    expect_contains(section_process.stdout_text, "\"deletedPreviewBoundsRight\": 0",
                    "#1987: record-selected deleted group-footer label section JSON should preserve deleted preview right bounds");
    expect_contains(section_process.stdout_text, "\"deletedPreviewBoundsBottom\": 4100",
                    "#1987: record-selected deleted group-footer label section JSON should preserve deleted preview bottom bounds");
    expect_contains(section_process.stdout_text, "\"deletedPreviewBoundsWidth\": 0",
                    "#1987: record-selected deleted group-footer label section JSON should preserve deleted preview widths");
    expect_contains(section_process.stdout_text, "\"deletedPreviewBoundsHeight\": 500",
                    "#1987: record-selected deleted group-footer label section JSON should preserve deleted preview heights");
    expect_contains(section_process.stdout_text, "\"selectedReportObjectAvailable\": false",
                    "#1987: record-selected deleted group-footer label sections should not advertise selected-object availability");
    expect_contains(section_process.stdout_text, "\"selectedReportObject\": null",
                    "#1987: record-selected deleted group-footer label sections should serialize null selected objects");
    expect_contains(section_process.stdout_text, "\"selectedReportObjectSectionAvailable\": false",
                    "#1987: record-selected deleted group-footer label sections should not advertise selected object-section availability");
    expect_contains(section_process.stdout_text, "\"selectedReportObjectSection\": null",
                    "#1987: record-selected deleted group-footer label sections should serialize null selected object sections");
    expect_contains(section_process.stdout_text, "\"selectedReportSettingsAvailable\": false",
                    "#1987: record-selected deleted group-footer label sections should not advertise selected-settings availability");
    expect_contains(section_process.stdout_text, "\"selectedReportSettings\": null",
                    "#1987: record-selected deleted group-footer label sections should serialize null selected settings");
    expect_contains(section_process.stdout_text, "\"sectionCount\": 2",
                    "#1987: record-selected deleted group-footer label section JSON should preserve live section counts");
    expect_contains(section_process.stdout_text, "\"deletedSectionCount\": 1",
                    "#1987: record-selected deleted group-footer label section JSON should expose deleted section counts");
    expect_contains_in_order(
        section_process.stdout_text,
        {
            "\"deletedSections\": [",
            "\"id\": \"group_footer_3\"",
            "\"bandKind\": \"group_footer\"",
            "\"expression\": \"customer.country\"",
            "\"expressionFieldIndex\": 2",
            "\"expressionMemoBlockNumber\": 3",
            "\"recordIndex\": 3",
            "\"deleted\": true"
        },
        "#1987: record-selected deleted group-footer label section JSON should expose deleted section metadata");
    expect_contains_in_order(
        section_process.stdout_text,
        {
            "\"selectedReportSection\": {",
            "\"id\": \"group_footer_3\"",
            "\"bandKind\": \"group_footer\"",
            "\"expression\": \"customer.country\"",
            "\"expressionFieldIndex\": 2",
            "\"expressionMemoBlockNumber\": 3",
            "\"recordIndex\": 3",
            "\"deleted\": true",
            "\"sectionIndex\": null",
            "\"sectionCount\": 0",
            "\"top\": 3600",
            "\"height\": 500",
            "\"bottom\": 4100"
        },
        "#1987: record-selected deleted group-footer label sections should expose selected expression metadata");
    expect_contains_in_order(
        section_process.stdout_text,
        {
            "\"sections\": [",
            "\"bandKind\": \"group_header\"",
            "\"recordIndex\": 1",
            "\"bandKind\": \"detail\"",
            "\"recordIndex\": 2"
        },
        "#1987: record-selected deleted group-footer label section JSON should preserve live sibling metadata");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_exposes_selected_summary_label_sections_by_record_selection(
    const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_selected_summary_label_sections_record_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path label_path = temp_root / "selected_summary_section_record.lbx";
    write_synthetic_report_table_for_stable_summary_section_json(label_path);

    const auto section_process = run_process_capture(
        studio_host_path,
        {"--path", label_path.string(), "--record", "2", "--json"},
        temp_root);

    if (section_process.exit_code != 0) {
        std::cerr << "studio host record-selected summary label section stdout:\n"
                  << section_process.stdout_text << "\n";
        std::cerr << "studio host record-selected summary label section stderr:\n"
                  << section_process.stderr_text << "\n";
        std::cerr << "fixture root: " << temp_root << "\n";
    }

    expect(section_process.exit_code == 0,
           "#1989: record-selected summary label section JSON should exit successfully");
    expect_contains(section_process.stdout_text,
                    "\"documentTitle\": \"selected_summary_section_record.lbx\"",
                    "#1989: record-selected summary label section JSON should preserve document titles");
    expect_contains(section_process.stdout_text, "\"isLabel\": true",
                    "#1989: record-selected summary label section JSON should retain label identity");
    expect_contains(section_process.stdout_text, "\"selectedReportSectionAvailable\": true",
                    "#1989: record-selected summary label sections should advertise selected-section availability");
    expect_contains(section_process.stdout_text, "\"selectedReportSelectionAvailable\": true",
                    "#1989: record-selected summary label sections should advertise report-selection availability");
    expect_contains(section_process.stdout_text, "\"selectedReportSelectionKind\": \"section\"",
                    "#1989: record-selected summary label sections should expose section selection kind");
    expect_contains(section_process.stdout_text, "\"previewBoundsAvailable\": true",
                    "#1989: record-selected summary label section JSON should expose live preview availability");
    expect_contains(section_process.stdout_text, "\"previewBoundsLeft\": 0",
                    "#1989: record-selected summary label section JSON should preserve live preview left bounds");
    expect_contains(section_process.stdout_text, "\"previewBoundsTop\": 0",
                    "#1989: record-selected summary label section JSON should preserve live preview top bounds");
    expect_contains(section_process.stdout_text, "\"previewBoundsRight\": 0",
                    "#1989: record-selected summary label section JSON should preserve live preview right bounds");
    expect_contains(section_process.stdout_text, "\"previewBoundsBottom\": 3900",
                    "#1989: record-selected summary label section JSON should preserve live preview bottom bounds");
    expect_contains(section_process.stdout_text, "\"previewBoundsWidth\": 0",
                    "#1989: record-selected summary label section JSON should preserve live preview widths");
    expect_contains(section_process.stdout_text, "\"previewBoundsHeight\": 3900",
                    "#1989: record-selected summary label section JSON should preserve live preview heights");
    expect_contains(section_process.stdout_text, "\"deletedPreviewBoundsAvailable\": false",
                    "#1989: record-selected summary label section JSON should not fabricate deleted preview bounds");
    expect_contains(section_process.stdout_text, "\"selectedReportObjectAvailable\": false",
                    "#1989: record-selected summary label sections should not advertise selected-object availability");
    expect_contains(section_process.stdout_text, "\"selectedReportObject\": null",
                    "#1989: record-selected summary label sections should serialize null selected objects");
    expect_contains(section_process.stdout_text, "\"selectedReportObjectSectionAvailable\": false",
                    "#1989: record-selected summary label sections should not advertise selected object-section availability");
    expect_contains(section_process.stdout_text, "\"selectedReportObjectSection\": null",
                    "#1989: record-selected summary label sections should serialize null selected object sections");
    expect_contains(section_process.stdout_text, "\"selectedReportSettingsAvailable\": false",
                    "#1989: record-selected summary label sections should not advertise selected-settings availability");
    expect_contains(section_process.stdout_text, "\"selectedReportSettings\": null",
                    "#1989: record-selected summary label sections should serialize null selected settings");
    expect_contains(section_process.stdout_text, "\"sectionCount\": 2",
                    "#1989: record-selected summary label section JSON should preserve live section counts");
    expect_contains(section_process.stdout_text, "\"deletedSectionCount\": 0",
                    "#1989: record-selected summary label section JSON should preserve deleted section counts");
    expect_contains_in_order(
        section_process.stdout_text,
        {
            "\"sections\": [",
            "\"bandKind\": \"detail\"",
            "\"recordIndex\": 1",
            "\"bandKind\": \"summary\"",
            "\"recordIndex\": 2"
        },
        "#1989: record-selected summary label section JSON should expose sibling section metadata");
    expect_contains_in_order(
        section_process.stdout_text,
        {
            "\"selectedReportSection\": {",
            "\"id\": \"summary_2\"",
            "\"bandKind\": \"summary\"",
            "\"recordIndex\": 2",
            "\"deleted\": false",
            "\"sectionIndex\": 1",
            "\"sectionCount\": 2",
            "\"top\": 3200",
            "\"height\": 700",
            "\"bottom\": 3900",
            "\"objectCount\": 0"
        },
        "#1989: record-selected summary label sections should expose selected summary metadata");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_exposes_selected_deleted_summary_label_sections_by_record_selection(
    const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_selected_deleted_summary_label_sections_record_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path label_path = temp_root / "selected_deleted_summary_section_record.lbx";
    write_synthetic_report_table_for_stable_summary_section_json(label_path);
    const auto delete_result = copperfin::vfp::set_record_deleted_flag(label_path.string(), 2U, true);
    expect(delete_result.ok && dbf_record_deleted(label_path, 2U),
           "#1991: record-selected deleted summary label fixture should mark the summary section deleted");

    const auto section_process = run_process_capture(
        studio_host_path,
        {"--path", label_path.string(), "--record", "2", "--json"},
        temp_root);

    if (section_process.exit_code != 0) {
        std::cerr << "studio host record-selected deleted summary label section stdout:\n"
                  << section_process.stdout_text << "\n";
        std::cerr << "studio host record-selected deleted summary label section stderr:\n"
                  << section_process.stderr_text << "\n";
        std::cerr << "fixture root: " << temp_root << "\n";
    }

    expect(section_process.exit_code == 0,
           "#1991: record-selected deleted summary label section JSON should exit successfully");
    expect_contains(section_process.stdout_text,
                    "\"documentTitle\": \"selected_deleted_summary_section_record.lbx\"",
                    "#1991: record-selected deleted summary label section JSON should preserve document titles");
    expect_contains(section_process.stdout_text, "\"isLabel\": true",
                    "#1991: record-selected deleted summary label section JSON should retain label identity");
    expect_contains(section_process.stdout_text, "\"selectedReportSectionAvailable\": true",
                    "#1991: record-selected deleted summary label sections should advertise selected-section availability");
    expect_contains(section_process.stdout_text, "\"selectedReportSelectionAvailable\": true",
                    "#1991: record-selected deleted summary label sections should advertise report-selection availability");
    expect_contains(section_process.stdout_text, "\"selectedReportSelectionKind\": \"section\"",
                    "#1991: record-selected deleted summary label sections should expose section selection kind");
    expect_contains(section_process.stdout_text, "\"previewBoundsAvailable\": true",
                    "#1991: record-selected deleted summary label section JSON should expose live preview availability");
    expect_contains(section_process.stdout_text, "\"previewBoundsLeft\": 0",
                    "#1991: record-selected deleted summary label section JSON should preserve live preview left bounds");
    expect_contains(section_process.stdout_text, "\"previewBoundsTop\": 0",
                    "#1991: record-selected deleted summary label section JSON should preserve live preview top bounds");
    expect_contains(section_process.stdout_text, "\"previewBoundsRight\": 0",
                    "#1991: record-selected deleted summary label section JSON should preserve live preview right bounds");
    expect_contains(section_process.stdout_text, "\"previewBoundsBottom\": 3200",
                    "#1991: record-selected deleted summary label section JSON should refresh live preview bottom bounds");
    expect_contains(section_process.stdout_text, "\"previewBoundsWidth\": 0",
                    "#1991: record-selected deleted summary label section JSON should preserve live preview widths");
    expect_contains(section_process.stdout_text, "\"previewBoundsHeight\": 3200",
                    "#1991: record-selected deleted summary label section JSON should refresh live preview heights");
    expect_contains(section_process.stdout_text, "\"deletedPreviewBoundsAvailable\": true",
                    "#1991: record-selected deleted summary label section JSON should expose deleted preview availability");
    expect_contains(section_process.stdout_text, "\"deletedPreviewBoundsLeft\": 0",
                    "#1991: record-selected deleted summary label section JSON should preserve deleted preview left bounds");
    expect_contains(section_process.stdout_text, "\"deletedPreviewBoundsTop\": 3200",
                    "#1991: record-selected deleted summary label section JSON should preserve deleted preview top bounds");
    expect_contains(section_process.stdout_text, "\"deletedPreviewBoundsRight\": 0",
                    "#1991: record-selected deleted summary label section JSON should preserve deleted preview right bounds");
    expect_contains(section_process.stdout_text, "\"deletedPreviewBoundsBottom\": 3900",
                    "#1991: record-selected deleted summary label section JSON should preserve deleted preview bottom bounds");
    expect_contains(section_process.stdout_text, "\"deletedPreviewBoundsWidth\": 0",
                    "#1991: record-selected deleted summary label section JSON should preserve deleted preview widths");
    expect_contains(section_process.stdout_text, "\"deletedPreviewBoundsHeight\": 700",
                    "#1991: record-selected deleted summary label section JSON should preserve deleted preview heights");
    expect_contains(section_process.stdout_text, "\"selectedReportObjectAvailable\": false",
                    "#1991: record-selected deleted summary label sections should not advertise selected-object availability");
    expect_contains(section_process.stdout_text, "\"selectedReportObject\": null",
                    "#1991: record-selected deleted summary label sections should serialize null selected objects");
    expect_contains(section_process.stdout_text, "\"selectedReportObjectSectionAvailable\": false",
                    "#1991: record-selected deleted summary label sections should not advertise selected object-section availability");
    expect_contains(section_process.stdout_text, "\"selectedReportObjectSection\": null",
                    "#1991: record-selected deleted summary label sections should serialize null selected object sections");
    expect_contains(section_process.stdout_text, "\"selectedReportSettingsAvailable\": false",
                    "#1991: record-selected deleted summary label sections should not advertise selected-settings availability");
    expect_contains(section_process.stdout_text, "\"selectedReportSettings\": null",
                    "#1991: record-selected deleted summary label sections should serialize null selected settings");
    expect_contains(section_process.stdout_text, "\"sectionCount\": 1",
                    "#1991: record-selected deleted summary label section JSON should preserve live section counts");
    expect_contains(section_process.stdout_text, "\"deletedSectionCount\": 1",
                    "#1991: record-selected deleted summary label section JSON should expose deleted section counts");
    expect_contains_in_order(
        section_process.stdout_text,
        {
            "\"deletedSections\": [",
            "\"id\": \"summary_2\"",
            "\"bandKind\": \"summary\"",
            "\"recordIndex\": 2",
            "\"deleted\": true"
        },
        "#1991: record-selected deleted summary label section JSON should expose deleted summary metadata");
    expect_contains_in_order(
        section_process.stdout_text,
        {
            "\"selectedReportSection\": {",
            "\"id\": \"summary_2\"",
            "\"bandKind\": \"summary\"",
            "\"recordIndex\": 2",
            "\"deleted\": true",
            "\"sectionIndex\": null",
            "\"sectionCount\": 0",
            "\"top\": 3200",
            "\"height\": 700",
            "\"bottom\": 3900"
        },
        "#1991: record-selected deleted summary label sections should expose selected deleted summary metadata");
    expect_contains_in_order(
        section_process.stdout_text,
        {
            "\"sections\": [",
            "\"bandKind\": \"detail\"",
            "\"recordIndex\": 1"
        },
        "#1991: record-selected deleted summary label section JSON should preserve live detail metadata");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_exposes_selected_title_label_sections_by_record_selection(
    const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_selected_title_label_sections_record_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path label_path = temp_root / "selected_title_section_record.lbx";
    write_synthetic_report_table_for_stable_title_section_json(label_path);

    const auto section_process = run_process_capture(
        studio_host_path,
        {"--path", label_path.string(), "--record", "1", "--json"},
        temp_root);

    if (section_process.exit_code != 0) {
        std::cerr << "studio host record-selected title label section stdout:\n"
                  << section_process.stdout_text << "\n";
        std::cerr << "studio host record-selected title label section stderr:\n"
                  << section_process.stderr_text << "\n";
        std::cerr << "fixture root: " << temp_root << "\n";
    }

    expect(section_process.exit_code == 0,
           "#1993: record-selected title label section JSON should exit successfully");
    expect_contains(section_process.stdout_text,
                    "\"documentTitle\": \"selected_title_section_record.lbx\"",
                    "#1993: record-selected title label section JSON should preserve document titles");
    expect_contains(section_process.stdout_text, "\"isLabel\": true",
                    "#1993: record-selected title label section JSON should retain label identity");
    expect_contains(section_process.stdout_text, "\"selectedReportSectionAvailable\": true",
                    "#1993: record-selected title label sections should advertise selected-section availability");
    expect_contains(section_process.stdout_text, "\"selectedReportSelectionAvailable\": true",
                    "#1993: record-selected title label sections should advertise report-selection availability");
    expect_contains(section_process.stdout_text, "\"selectedReportSelectionKind\": \"section\"",
                    "#1993: record-selected title label sections should expose section selection kind");
    expect_contains(section_process.stdout_text, "\"previewBoundsAvailable\": true",
                    "#1993: record-selected title label section JSON should expose live preview availability");
    expect_contains(section_process.stdout_text, "\"previewBoundsLeft\": 0",
                    "#1993: record-selected title label section JSON should preserve live preview left bounds");
    expect_contains(section_process.stdout_text, "\"previewBoundsTop\": 0",
                    "#1993: record-selected title label section JSON should preserve live preview top bounds");
    expect_contains(section_process.stdout_text, "\"previewBoundsRight\": 0",
                    "#1993: record-selected title label section JSON should preserve live preview right bounds");
    expect_contains(section_process.stdout_text, "\"previewBoundsBottom\": 3700",
                    "#1993: record-selected title label section JSON should preserve live preview bottom bounds");
    expect_contains(section_process.stdout_text, "\"previewBoundsWidth\": 0",
                    "#1993: record-selected title label section JSON should preserve live preview widths");
    expect_contains(section_process.stdout_text, "\"previewBoundsHeight\": 3700",
                    "#1993: record-selected title label section JSON should preserve live preview heights");
    expect_contains(section_process.stdout_text, "\"deletedPreviewBoundsAvailable\": false",
                    "#1993: record-selected title label section JSON should not fabricate deleted preview availability");
    expect_contains(section_process.stdout_text, "\"selectedReportObjectAvailable\": false",
                    "#1993: record-selected title label sections should not advertise selected-object availability");
    expect_contains(section_process.stdout_text, "\"selectedReportObject\": null",
                    "#1993: record-selected title label sections should serialize null selected objects");
    expect_contains(section_process.stdout_text, "\"selectedReportObjectSectionAvailable\": false",
                    "#1993: record-selected title label sections should not advertise selected object-section availability");
    expect_contains(section_process.stdout_text, "\"selectedReportObjectSection\": null",
                    "#1993: record-selected title label sections should serialize null selected object sections");
    expect_contains(section_process.stdout_text, "\"selectedReportSettingsAvailable\": false",
                    "#1993: record-selected title label sections should not advertise selected-settings availability");
    expect_contains(section_process.stdout_text, "\"selectedReportSettings\": null",
                    "#1993: record-selected title label sections should serialize null selected settings");
    expect_contains(section_process.stdout_text, "\"sectionCount\": 3",
                    "#1993: record-selected title label section JSON should preserve live section counts");
    expect_contains(section_process.stdout_text, "\"deletedSectionCount\": 0",
                    "#1993: record-selected title label section JSON should preserve deleted section counts");
    expect_contains_in_order(
        section_process.stdout_text,
        {
            "\"sections\": [",
            "\"bandKind\": \"title\"",
            "\"recordIndex\": 1",
            "\"bandKind\": \"detail\"",
            "\"recordIndex\": 2",
            "\"bandKind\": \"page_footer\"",
            "\"recordIndex\": 3"
        },
        "#1993: record-selected title label section JSON should expose sibling section metadata");
    expect_contains_in_order(
        section_process.stdout_text,
        {
            "\"selectedReportSection\": {",
            "\"id\": \"title_1\"",
            "\"bandKind\": \"title\"",
            "\"recordIndex\": 1",
            "\"deleted\": false",
            "\"sectionIndex\": 0",
            "\"sectionCount\": 3",
            "\"top\": 0",
            "\"height\": 700",
            "\"bottom\": 700"
        },
        "#1993: record-selected title label sections should expose selected section metadata");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_exposes_selected_deleted_title_label_sections_by_record_selection(
    const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_selected_deleted_title_label_sections_record_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path label_path = temp_root / "selected_deleted_title_section_record.lbx";
    write_synthetic_report_table_for_stable_title_section_json(label_path);
    const auto delete_result = copperfin::vfp::set_record_deleted_flag(label_path.string(), 1U, true);
    expect(delete_result.ok && dbf_record_deleted(label_path, 1U),
           "#1995: record-selected deleted title label fixture should mark the title section deleted");

    const auto section_process = run_process_capture(
        studio_host_path,
        {"--path", label_path.string(), "--record", "1", "--json"},
        temp_root);

    if (section_process.exit_code != 0) {
        std::cerr << "studio host record-selected deleted title label section stdout:\n"
                  << section_process.stdout_text << "\n";
        std::cerr << "studio host record-selected deleted title label section stderr:\n"
                  << section_process.stderr_text << "\n";
        std::cerr << "fixture root: " << temp_root << "\n";
    }

    expect(section_process.exit_code == 0,
           "#1995: record-selected deleted title label section JSON should exit successfully");
    expect_contains(section_process.stdout_text,
                    "\"documentTitle\": \"selected_deleted_title_section_record.lbx\"",
                    "#1995: record-selected deleted title label section JSON should preserve document titles");
    expect_contains(section_process.stdout_text, "\"isLabel\": true",
                    "#1995: record-selected deleted title label section JSON should retain label identity");
    expect_contains(section_process.stdout_text, "\"selectedReportSectionAvailable\": true",
                    "#1995: record-selected deleted title label sections should advertise selected-section availability");
    expect_contains(section_process.stdout_text, "\"selectedReportSelectionAvailable\": true",
                    "#1995: record-selected deleted title label sections should advertise report-selection availability");
    expect_contains(section_process.stdout_text, "\"selectedReportSelectionKind\": \"section\"",
                    "#1995: record-selected deleted title label sections should expose section selection kind");
    expect_contains(section_process.stdout_text, "\"previewBoundsAvailable\": true",
                    "#1995: record-selected deleted title label section JSON should expose live preview availability");
    expect_contains(section_process.stdout_text, "\"previewBoundsLeft\": 0",
                    "#1995: record-selected deleted title label section JSON should preserve live preview left bounds");
    expect_contains(section_process.stdout_text, "\"previewBoundsTop\": 700",
                    "#1995: record-selected deleted title label section JSON should refresh live preview top bounds");
    expect_contains(section_process.stdout_text, "\"previewBoundsRight\": 0",
                    "#1995: record-selected deleted title label section JSON should preserve live preview right bounds");
    expect_contains(section_process.stdout_text, "\"previewBoundsBottom\": 3700",
                    "#1995: record-selected deleted title label section JSON should preserve live preview bottom bounds");
    expect_contains(section_process.stdout_text, "\"previewBoundsWidth\": 0",
                    "#1995: record-selected deleted title label section JSON should preserve live preview widths");
    expect_contains(section_process.stdout_text, "\"previewBoundsHeight\": 3000",
                    "#1995: record-selected deleted title label section JSON should refresh live preview heights");
    expect_contains(section_process.stdout_text, "\"deletedPreviewBoundsAvailable\": true",
                    "#1995: record-selected deleted title label section JSON should expose deleted preview availability");
    expect_contains(section_process.stdout_text, "\"deletedPreviewBoundsLeft\": 0",
                    "#1995: record-selected deleted title label section JSON should preserve deleted preview left bounds");
    expect_contains(section_process.stdout_text, "\"deletedPreviewBoundsTop\": 0",
                    "#1995: record-selected deleted title label section JSON should preserve deleted preview top bounds");
    expect_contains(section_process.stdout_text, "\"deletedPreviewBoundsRight\": 0",
                    "#1995: record-selected deleted title label section JSON should preserve deleted preview right bounds");
    expect_contains(section_process.stdout_text, "\"deletedPreviewBoundsBottom\": 700",
                    "#1995: record-selected deleted title label section JSON should preserve deleted preview bottom bounds");
    expect_contains(section_process.stdout_text, "\"deletedPreviewBoundsWidth\": 0",
                    "#1995: record-selected deleted title label section JSON should preserve deleted preview widths");
    expect_contains(section_process.stdout_text, "\"deletedPreviewBoundsHeight\": 700",
                    "#1995: record-selected deleted title label section JSON should preserve deleted preview heights");
    expect_contains(section_process.stdout_text, "\"selectedReportObjectAvailable\": false",
                    "#1995: record-selected deleted title label sections should not advertise selected-object availability");
    expect_contains(section_process.stdout_text, "\"selectedReportObject\": null",
                    "#1995: record-selected deleted title label sections should serialize null selected objects");
    expect_contains(section_process.stdout_text, "\"selectedReportObjectSectionAvailable\": false",
                    "#1995: record-selected deleted title label sections should not advertise selected object-section availability");
    expect_contains(section_process.stdout_text, "\"selectedReportObjectSection\": null",
                    "#1995: record-selected deleted title label sections should serialize null selected object sections");
    expect_contains(section_process.stdout_text, "\"selectedReportSettingsAvailable\": false",
                    "#1995: record-selected deleted title label sections should not advertise selected-settings availability");
    expect_contains(section_process.stdout_text, "\"selectedReportSettings\": null",
                    "#1995: record-selected deleted title label sections should serialize null selected settings");
    expect_contains(section_process.stdout_text, "\"sectionCount\": 2",
                    "#1995: record-selected deleted title label section JSON should preserve live section counts");
    expect_contains(section_process.stdout_text, "\"deletedSectionCount\": 1",
                    "#1995: record-selected deleted title label section JSON should preserve deleted section counts");
    expect_contains_in_order(
        section_process.stdout_text,
        {
            "\"deletedSections\": [",
            "\"id\": \"title_1\"",
            "\"bandKind\": \"title\"",
            "\"recordIndex\": 1",
            "\"deleted\": true"
        },
        "#1995: record-selected deleted title label section JSON should expose deleted title metadata");
    expect_contains_in_order(
        section_process.stdout_text,
        {
            "\"sections\": [",
            "\"bandKind\": \"detail\"",
            "\"recordIndex\": 2",
            "\"bandKind\": \"page_footer\"",
            "\"recordIndex\": 3"
        },
        "#1995: record-selected deleted title label section JSON should expose live sibling section metadata");
    expect_contains_in_order(
        section_process.stdout_text,
        {
            "\"selectedReportSection\": {",
            "\"id\": \"title_1\"",
            "\"bandKind\": \"title\"",
            "\"recordIndex\": 1",
            "\"deleted\": true",
            "\"sectionIndex\": null",
            "\"sectionCount\": 0",
            "\"top\": 0",
            "\"height\": 700",
            "\"bottom\": 700"
        },
        "#1995: record-selected deleted title label sections should expose selected deleted title metadata");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_exposes_selected_page_footer_label_sections_by_record_selection(
    const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_selected_page_footer_label_sections_record_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path label_path = temp_root / "selected_page_footer_section_record.lbx";
    write_synthetic_report_table_for_stable_title_section_json(label_path);

    const auto section_process = run_process_capture(
        studio_host_path,
        {"--path", label_path.string(), "--record", "3", "--json"},
        temp_root);

    if (section_process.exit_code != 0) {
        std::cerr << "studio host record-selected page-footer label section stdout:\n"
                  << section_process.stdout_text << "\n";
        std::cerr << "studio host record-selected page-footer label section stderr:\n"
                  << section_process.stderr_text << "\n";
        std::cerr << "fixture root: " << temp_root << "\n";
    }

    expect(section_process.exit_code == 0,
           "#1997: record-selected page-footer label section JSON should exit successfully");
    expect_contains(section_process.stdout_text,
                    "\"documentTitle\": \"selected_page_footer_section_record.lbx\"",
                    "#1997: record-selected page-footer label section JSON should preserve document titles");
    expect_contains(section_process.stdout_text, "\"isLabel\": true",
                    "#1997: record-selected page-footer label section JSON should retain label identity");
    expect_contains(section_process.stdout_text, "\"selectedReportSectionAvailable\": true",
                    "#1997: record-selected page-footer label sections should advertise selected-section availability");
    expect_contains(section_process.stdout_text, "\"selectedReportSelectionAvailable\": true",
                    "#1997: record-selected page-footer label sections should advertise report-selection availability");
    expect_contains(section_process.stdout_text, "\"selectedReportSelectionKind\": \"section\"",
                    "#1997: record-selected page-footer label sections should expose section selection kind");
    expect_contains(section_process.stdout_text, "\"previewBoundsAvailable\": true",
                    "#1997: record-selected page-footer label section JSON should expose live preview availability");
    expect_contains(section_process.stdout_text, "\"previewBoundsLeft\": 0",
                    "#1997: record-selected page-footer label section JSON should preserve live preview left bounds");
    expect_contains(section_process.stdout_text, "\"previewBoundsTop\": 0",
                    "#1997: record-selected page-footer label section JSON should preserve live preview top bounds");
    expect_contains(section_process.stdout_text, "\"previewBoundsRight\": 0",
                    "#1997: record-selected page-footer label section JSON should preserve live preview right bounds");
    expect_contains(section_process.stdout_text, "\"previewBoundsBottom\": 3700",
                    "#1997: record-selected page-footer label section JSON should preserve live preview bottom bounds");
    expect_contains(section_process.stdout_text, "\"previewBoundsWidth\": 0",
                    "#1997: record-selected page-footer label section JSON should preserve live preview widths");
    expect_contains(section_process.stdout_text, "\"previewBoundsHeight\": 3700",
                    "#1997: record-selected page-footer label section JSON should preserve live preview heights");
    expect_contains(section_process.stdout_text, "\"deletedPreviewBoundsAvailable\": false",
                    "#1997: record-selected page-footer label section JSON should not fabricate deleted preview availability");
    expect_contains(section_process.stdout_text, "\"selectedReportObjectAvailable\": false",
                    "#1997: record-selected page-footer label sections should not advertise selected-object availability");
    expect_contains(section_process.stdout_text, "\"selectedReportObject\": null",
                    "#1997: record-selected page-footer label sections should serialize null selected objects");
    expect_contains(section_process.stdout_text, "\"selectedReportObjectSectionAvailable\": false",
                    "#1997: record-selected page-footer label sections should not advertise selected object-section availability");
    expect_contains(section_process.stdout_text, "\"selectedReportObjectSection\": null",
                    "#1997: record-selected page-footer label sections should serialize null selected object sections");
    expect_contains(section_process.stdout_text, "\"selectedReportSettingsAvailable\": false",
                    "#1997: record-selected page-footer label sections should not advertise selected-settings availability");
    expect_contains(section_process.stdout_text, "\"selectedReportSettings\": null",
                    "#1997: record-selected page-footer label sections should serialize null selected settings");
    expect_contains(section_process.stdout_text, "\"sectionCount\": 3",
                    "#1997: record-selected page-footer label section JSON should preserve live section counts");
    expect_contains(section_process.stdout_text, "\"deletedSectionCount\": 0",
                    "#1997: record-selected page-footer label section JSON should preserve deleted section counts");
    expect_contains_in_order(
        section_process.stdout_text,
        {
            "\"sections\": [",
            "\"bandKind\": \"title\"",
            "\"recordIndex\": 1",
            "\"bandKind\": \"detail\"",
            "\"recordIndex\": 2",
            "\"bandKind\": \"page_footer\"",
            "\"recordIndex\": 3"
        },
        "#1997: record-selected page-footer label section JSON should expose sibling section metadata");
    expect_contains_in_order(
        section_process.stdout_text,
        {
            "\"selectedReportSection\": {",
            "\"id\": \"page_footer_3\"",
            "\"bandKind\": \"page_footer\"",
            "\"recordIndex\": 3",
            "\"deleted\": false",
            "\"sectionIndex\": 2",
            "\"sectionCount\": 3",
            "\"top\": 3200",
            "\"height\": 500",
            "\"bottom\": 3700"
        },
        "#1997: record-selected page-footer label sections should expose selected section metadata");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_exposes_selected_deleted_page_footer_label_sections_by_record_selection(
    const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_selected_deleted_page_footer_label_sections_record_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path label_path = temp_root / "selected_deleted_page_footer_section_record.lbx";
    write_synthetic_report_table_for_stable_title_section_json(label_path);
    const auto delete_result = copperfin::vfp::set_record_deleted_flag(label_path.string(), 3U, true);
    expect(delete_result.ok && dbf_record_deleted(label_path, 3U),
           "#1999: record-selected deleted page-footer label fixture should mark the page-footer section deleted");

    const auto section_process = run_process_capture(
        studio_host_path,
        {"--path", label_path.string(), "--record", "3", "--json"},
        temp_root);

    if (section_process.exit_code != 0) {
        std::cerr << "studio host record-selected deleted page-footer label section stdout:\n"
                  << section_process.stdout_text << "\n";
        std::cerr << "studio host record-selected deleted page-footer label section stderr:\n"
                  << section_process.stderr_text << "\n";
        std::cerr << "fixture root: " << temp_root << "\n";
    }

    expect(section_process.exit_code == 0,
           "#1999: record-selected deleted page-footer label section JSON should exit successfully");
    expect_contains(section_process.stdout_text,
                    "\"documentTitle\": \"selected_deleted_page_footer_section_record.lbx\"",
                    "#1999: record-selected deleted page-footer label section JSON should preserve document titles");
    expect_contains(section_process.stdout_text, "\"isLabel\": true",
                    "#1999: record-selected deleted page-footer label section JSON should retain label identity");
    expect_contains(section_process.stdout_text, "\"selectedReportSectionAvailable\": true",
                    "#1999: record-selected deleted page-footer label sections should advertise selected-section availability");
    expect_contains(section_process.stdout_text, "\"selectedReportSelectionAvailable\": true",
                    "#1999: record-selected deleted page-footer label sections should advertise report-selection availability");
    expect_contains(section_process.stdout_text, "\"selectedReportSelectionKind\": \"section\"",
                    "#1999: record-selected deleted page-footer label sections should expose section selection kind");
    expect_contains(section_process.stdout_text, "\"previewBoundsAvailable\": true",
                    "#1999: record-selected deleted page-footer label section JSON should expose live preview availability");
    expect_contains(section_process.stdout_text, "\"previewBoundsLeft\": 0",
                    "#1999: record-selected deleted page-footer label section JSON should preserve live preview left bounds");
    expect_contains(section_process.stdout_text, "\"previewBoundsTop\": 0",
                    "#1999: record-selected deleted page-footer label section JSON should preserve live preview top bounds");
    expect_contains(section_process.stdout_text, "\"previewBoundsRight\": 0",
                    "#1999: record-selected deleted page-footer label section JSON should preserve live preview right bounds");
    expect_contains(section_process.stdout_text, "\"previewBoundsBottom\": 3200",
                    "#1999: record-selected deleted page-footer label section JSON should refresh live preview bottom bounds");
    expect_contains(section_process.stdout_text, "\"previewBoundsWidth\": 0",
                    "#1999: record-selected deleted page-footer label section JSON should preserve live preview widths");
    expect_contains(section_process.stdout_text, "\"previewBoundsHeight\": 3200",
                    "#1999: record-selected deleted page-footer label section JSON should refresh live preview heights");
    expect_contains(section_process.stdout_text, "\"deletedPreviewBoundsAvailable\": true",
                    "#1999: record-selected deleted page-footer label section JSON should expose deleted preview availability");
    expect_contains(section_process.stdout_text, "\"deletedPreviewBoundsLeft\": 0",
                    "#1999: record-selected deleted page-footer label section JSON should preserve deleted preview left bounds");
    expect_contains(section_process.stdout_text, "\"deletedPreviewBoundsTop\": 3200",
                    "#1999: record-selected deleted page-footer label section JSON should preserve deleted preview top bounds");
    expect_contains(section_process.stdout_text, "\"deletedPreviewBoundsRight\": 0",
                    "#1999: record-selected deleted page-footer label section JSON should preserve deleted preview right bounds");
    expect_contains(section_process.stdout_text, "\"deletedPreviewBoundsBottom\": 3700",
                    "#1999: record-selected deleted page-footer label section JSON should preserve deleted preview bottom bounds");
    expect_contains(section_process.stdout_text, "\"deletedPreviewBoundsWidth\": 0",
                    "#1999: record-selected deleted page-footer label section JSON should preserve deleted preview widths");
    expect_contains(section_process.stdout_text, "\"deletedPreviewBoundsHeight\": 500",
                    "#1999: record-selected deleted page-footer label section JSON should preserve deleted preview heights");
    expect_contains(section_process.stdout_text, "\"selectedReportObjectAvailable\": false",
                    "#1999: record-selected deleted page-footer label sections should not advertise selected-object availability");
    expect_contains(section_process.stdout_text, "\"selectedReportObject\": null",
                    "#1999: record-selected deleted page-footer label sections should serialize null selected objects");
    expect_contains(section_process.stdout_text, "\"selectedReportObjectSectionAvailable\": false",
                    "#1999: record-selected deleted page-footer label sections should not advertise selected object-section availability");
    expect_contains(section_process.stdout_text, "\"selectedReportObjectSection\": null",
                    "#1999: record-selected deleted page-footer label sections should serialize null selected object sections");
    expect_contains(section_process.stdout_text, "\"selectedReportSettingsAvailable\": false",
                    "#1999: record-selected deleted page-footer label sections should not advertise selected-settings availability");
    expect_contains(section_process.stdout_text, "\"selectedReportSettings\": null",
                    "#1999: record-selected deleted page-footer label sections should serialize null selected settings");
    expect_contains(section_process.stdout_text, "\"sectionCount\": 2",
                    "#1999: record-selected deleted page-footer label section JSON should preserve live section counts");
    expect_contains(section_process.stdout_text, "\"deletedSectionCount\": 1",
                    "#1999: record-selected deleted page-footer label section JSON should preserve deleted section counts");
    expect_contains_in_order(
        section_process.stdout_text,
        {
            "\"deletedSections\": [",
            "\"id\": \"page_footer_3\"",
            "\"bandKind\": \"page_footer\"",
            "\"recordIndex\": 3",
            "\"deleted\": true"
        },
        "#1999: record-selected deleted page-footer label section JSON should expose deleted page-footer metadata");
    expect_contains_in_order(
        section_process.stdout_text,
        {
            "\"sections\": [",
            "\"bandKind\": \"title\"",
            "\"recordIndex\": 1",
            "\"bandKind\": \"detail\"",
            "\"recordIndex\": 2"
        },
        "#1999: record-selected deleted page-footer label section JSON should expose live sibling section metadata");
    expect_contains_in_order(
        section_process.stdout_text,
        {
            "\"selectedReportSection\": {",
            "\"id\": \"page_footer_3\"",
            "\"bandKind\": \"page_footer\"",
            "\"recordIndex\": 3",
            "\"deleted\": true",
            "\"sectionIndex\": null",
            "\"sectionCount\": 0",
            "\"top\": 3200",
            "\"height\": 500",
            "\"bottom\": 3700"
        },
        "#1999: record-selected deleted page-footer label sections should expose selected deleted page-footer metadata");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_exposes_selected_column_header_label_sections_by_record_selection(
    const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_selected_column_header_label_sections_record_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path label_path = temp_root / "selected_column_header_section_record.lbx";
    write_synthetic_report_table_for_stable_column_section_json(label_path);

    const auto section_process = run_process_capture(
        studio_host_path,
        {"--path", label_path.string(), "--record", "1", "--json"},
        temp_root);

    if (section_process.exit_code != 0) {
        std::cerr << "studio host record-selected column-header label section stdout:\n"
                  << section_process.stdout_text << "\n";
        std::cerr << "studio host record-selected column-header label section stderr:\n"
                  << section_process.stderr_text << "\n";
        std::cerr << "fixture root: " << temp_root << "\n";
    }

    expect(section_process.exit_code == 0,
           "#2001: record-selected column-header label section JSON should exit successfully");
    expect_contains(section_process.stdout_text,
                    "\"documentTitle\": \"selected_column_header_section_record.lbx\"",
                    "#2001: record-selected column-header label section JSON should preserve document titles");
    expect_contains(section_process.stdout_text, "\"isLabel\": true",
                    "#2001: record-selected column-header label section JSON should retain label identity");
    expect_contains(section_process.stdout_text, "\"selectedReportSectionAvailable\": true",
                    "#2001: record-selected column-header label sections should advertise selected-section availability");
    expect_contains(section_process.stdout_text, "\"selectedReportSelectionAvailable\": true",
                    "#2001: record-selected column-header label sections should advertise report-selection availability");
    expect_contains(section_process.stdout_text, "\"selectedReportSelectionKind\": \"section\"",
                    "#2001: record-selected column-header label sections should expose section selection kind");
    expect_contains(section_process.stdout_text, "\"previewBoundsAvailable\": true",
                    "#2001: record-selected column-header label section JSON should expose live preview availability");
    expect_contains(section_process.stdout_text, "\"previewBoundsLeft\": 0",
                    "#2001: record-selected column-header label section JSON should preserve live preview left bounds");
    expect_contains(section_process.stdout_text, "\"previewBoundsTop\": 0",
                    "#2001: record-selected column-header label section JSON should preserve live preview top bounds");
    expect_contains(section_process.stdout_text, "\"previewBoundsRight\": 0",
                    "#2001: record-selected column-header label section JSON should preserve live preview right bounds");
    expect_contains(section_process.stdout_text, "\"previewBoundsBottom\": 3450",
                    "#2001: record-selected column-header label section JSON should preserve live preview bottom bounds");
    expect_contains(section_process.stdout_text, "\"previewBoundsWidth\": 0",
                    "#2001: record-selected column-header label section JSON should preserve live preview widths");
    expect_contains(section_process.stdout_text, "\"previewBoundsHeight\": 3450",
                    "#2001: record-selected column-header label section JSON should preserve live preview heights");
    expect_contains(section_process.stdout_text, "\"deletedPreviewBoundsAvailable\": false",
                    "#2001: record-selected column-header label section JSON should not fabricate deleted preview availability");
    expect_contains(section_process.stdout_text, "\"selectedReportObjectAvailable\": false",
                    "#2001: record-selected column-header label sections should not advertise selected-object availability");
    expect_contains(section_process.stdout_text, "\"selectedReportObject\": null",
                    "#2001: record-selected column-header label sections should serialize null selected objects");
    expect_contains(section_process.stdout_text, "\"selectedReportObjectSectionAvailable\": false",
                    "#2001: record-selected column-header label sections should not advertise selected object-section availability");
    expect_contains(section_process.stdout_text, "\"selectedReportObjectSection\": null",
                    "#2001: record-selected column-header label sections should serialize null selected object sections");
    expect_contains(section_process.stdout_text, "\"selectedReportSettingsAvailable\": false",
                    "#2001: record-selected column-header label sections should not advertise selected-settings availability");
    expect_contains(section_process.stdout_text, "\"selectedReportSettings\": null",
                    "#2001: record-selected column-header label sections should serialize null selected settings");
    expect_contains(section_process.stdout_text, "\"sectionCount\": 3",
                    "#2001: record-selected column-header label section JSON should preserve live section counts");
    expect_contains(section_process.stdout_text, "\"deletedSectionCount\": 0",
                    "#2001: record-selected column-header label section JSON should preserve deleted section counts");
    expect_contains_in_order(
        section_process.stdout_text,
        {
            "\"sections\": [",
            "\"bandKind\": \"column_header\"",
            "\"recordIndex\": 1",
            "\"bandKind\": \"detail\"",
            "\"recordIndex\": 2",
            "\"bandKind\": \"column_footer\"",
            "\"recordIndex\": 3"
        },
        "#2001: record-selected column-header label section JSON should expose sibling section metadata");
    expect_contains_in_order(
        section_process.stdout_text,
        {
            "\"selectedReportSection\": {",
            "\"id\": \"column_header_1\"",
            "\"bandKind\": \"column_header\"",
            "\"recordIndex\": 1",
            "\"deleted\": false",
            "\"sectionIndex\": 0",
            "\"sectionCount\": 3",
            "\"top\": 0",
            "\"height\": 450",
            "\"bottom\": 450"
        },
        "#2001: record-selected column-header label sections should expose selected section metadata");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_exposes_selected_deleted_column_header_label_sections_by_record_selection(
    const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_selected_deleted_column_header_label_sections_record_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path label_path = temp_root / "selected_deleted_column_header_section_record.lbx";
    write_synthetic_report_table_for_stable_column_section_json(label_path);
    const auto delete_result = copperfin::vfp::set_record_deleted_flag(label_path.string(), 1U, true);
    expect(delete_result.ok && dbf_record_deleted(label_path, 1U),
           "#2003: record-selected deleted column-header label fixture should mark the column-header section deleted");

    const auto section_process = run_process_capture(
        studio_host_path,
        {"--path", label_path.string(), "--record", "1", "--json"},
        temp_root);

    if (section_process.exit_code != 0) {
        std::cerr << "studio host record-selected deleted column-header label section stdout:\n"
                  << section_process.stdout_text << "\n";
        std::cerr << "studio host record-selected deleted column-header label section stderr:\n"
                  << section_process.stderr_text << "\n";
        std::cerr << "fixture root: " << temp_root << "\n";
    }

    expect(section_process.exit_code == 0,
           "#2003: record-selected deleted column-header label section JSON should exit successfully");
    expect_contains(section_process.stdout_text,
                    "\"documentTitle\": \"selected_deleted_column_header_section_record.lbx\"",
                    "#2003: record-selected deleted column-header label section JSON should preserve document titles");
    expect_contains(section_process.stdout_text, "\"isLabel\": true",
                    "#2003: record-selected deleted column-header label section JSON should retain label identity");
    expect_contains(section_process.stdout_text, "\"selectedReportSectionAvailable\": true",
                    "#2003: record-selected deleted column-header label sections should advertise selected-section availability");
    expect_contains(section_process.stdout_text, "\"selectedReportSelectionAvailable\": true",
                    "#2003: record-selected deleted column-header label sections should advertise report-selection availability");
    expect_contains(section_process.stdout_text, "\"selectedReportSelectionKind\": \"section\"",
                    "#2003: record-selected deleted column-header label sections should expose section selection kind");
    expect_contains(section_process.stdout_text, "\"previewBoundsAvailable\": true",
                    "#2003: record-selected deleted column-header label section JSON should expose live preview availability");
    expect_contains(section_process.stdout_text, "\"previewBoundsLeft\": 0",
                    "#2003: record-selected deleted column-header label section JSON should preserve live preview left bounds");
    expect_contains(section_process.stdout_text, "\"previewBoundsTop\": 450",
                    "#2003: record-selected deleted column-header label section JSON should refresh live preview top bounds");
    expect_contains(section_process.stdout_text, "\"previewBoundsRight\": 0",
                    "#2003: record-selected deleted column-header label section JSON should preserve live preview right bounds");
    expect_contains(section_process.stdout_text, "\"previewBoundsBottom\": 3450",
                    "#2003: record-selected deleted column-header label section JSON should preserve live preview bottom bounds");
    expect_contains(section_process.stdout_text, "\"previewBoundsWidth\": 0",
                    "#2003: record-selected deleted column-header label section JSON should preserve live preview widths");
    expect_contains(section_process.stdout_text, "\"previewBoundsHeight\": 3000",
                    "#2003: record-selected deleted column-header label section JSON should refresh live preview heights");
    expect_contains(section_process.stdout_text, "\"deletedPreviewBoundsAvailable\": true",
                    "#2003: record-selected deleted column-header label section JSON should expose deleted preview availability");
    expect_contains(section_process.stdout_text, "\"deletedPreviewBoundsLeft\": 0",
                    "#2003: record-selected deleted column-header label section JSON should preserve deleted preview left bounds");
    expect_contains(section_process.stdout_text, "\"deletedPreviewBoundsTop\": 0",
                    "#2003: record-selected deleted column-header label section JSON should preserve deleted preview top bounds");
    expect_contains(section_process.stdout_text, "\"deletedPreviewBoundsRight\": 0",
                    "#2003: record-selected deleted column-header label section JSON should preserve deleted preview right bounds");
    expect_contains(section_process.stdout_text, "\"deletedPreviewBoundsBottom\": 450",
                    "#2003: record-selected deleted column-header label section JSON should preserve deleted preview bottom bounds");
    expect_contains(section_process.stdout_text, "\"deletedPreviewBoundsWidth\": 0",
                    "#2003: record-selected deleted column-header label section JSON should preserve deleted preview widths");
    expect_contains(section_process.stdout_text, "\"deletedPreviewBoundsHeight\": 450",
                    "#2003: record-selected deleted column-header label section JSON should preserve deleted preview heights");
    expect_contains(section_process.stdout_text, "\"selectedReportObjectAvailable\": false",
                    "#2003: record-selected deleted column-header label sections should not advertise selected-object availability");
    expect_contains(section_process.stdout_text, "\"selectedReportObject\": null",
                    "#2003: record-selected deleted column-header label sections should serialize null selected objects");
    expect_contains(section_process.stdout_text, "\"selectedReportObjectSectionAvailable\": false",
                    "#2003: record-selected deleted column-header label sections should not advertise selected object-section availability");
    expect_contains(section_process.stdout_text, "\"selectedReportObjectSection\": null",
                    "#2003: record-selected deleted column-header label sections should serialize null selected object sections");
    expect_contains(section_process.stdout_text, "\"selectedReportSettingsAvailable\": false",
                    "#2003: record-selected deleted column-header label sections should not advertise selected-settings availability");
    expect_contains(section_process.stdout_text, "\"selectedReportSettings\": null",
                    "#2003: record-selected deleted column-header label sections should serialize null selected settings");
    expect_contains(section_process.stdout_text, "\"sectionCount\": 2",
                    "#2003: record-selected deleted column-header label section JSON should preserve live section counts");
    expect_contains(section_process.stdout_text, "\"deletedSectionCount\": 1",
                    "#2003: record-selected deleted column-header label section JSON should preserve deleted section counts");
    expect_contains_in_order(
        section_process.stdout_text,
        {
            "\"deletedSections\": [",
            "\"id\": \"column_header_1\"",
            "\"bandKind\": \"column_header\"",
            "\"recordIndex\": 1",
            "\"deleted\": true"
        },
        "#2003: record-selected deleted column-header label section JSON should expose deleted column-header metadata");
    expect_contains_in_order(
        section_process.stdout_text,
        {
            "\"sections\": [",
            "\"bandKind\": \"detail\"",
            "\"recordIndex\": 2",
            "\"bandKind\": \"column_footer\"",
            "\"recordIndex\": 3"
        },
        "#2003: record-selected deleted column-header label section JSON should expose live sibling section metadata");
    expect_contains_in_order(
        section_process.stdout_text,
        {
            "\"selectedReportSection\": {",
            "\"id\": \"column_header_1\"",
            "\"bandKind\": \"column_header\"",
            "\"recordIndex\": 1",
            "\"deleted\": true",
            "\"sectionIndex\": null",
            "\"sectionCount\": 0",
            "\"top\": 0",
            "\"height\": 450",
            "\"bottom\": 450"
        },
        "#2003: record-selected deleted column-header label sections should expose selected deleted column-header metadata");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_exposes_selected_column_footer_label_sections_by_record_selection(
    const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_selected_column_footer_label_sections_record_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path label_path = temp_root / "selected_column_footer_section_record.lbx";
    write_synthetic_report_table_for_stable_column_section_json(label_path);

    const auto section_process = run_process_capture(
        studio_host_path,
        {"--path", label_path.string(), "--record", "3", "--json"},
        temp_root);

    if (section_process.exit_code != 0) {
        std::cerr << "studio host record-selected column-footer label section stdout:\n"
                  << section_process.stdout_text << "\n";
        std::cerr << "studio host record-selected column-footer label section stderr:\n"
                  << section_process.stderr_text << "\n";
        std::cerr << "fixture root: " << temp_root << "\n";
    }

    expect(section_process.exit_code == 0,
           "#2005: record-selected column-footer label section JSON should exit successfully");
    expect_contains(section_process.stdout_text,
                    "\"documentTitle\": \"selected_column_footer_section_record.lbx\"",
                    "#2005: record-selected column-footer label section JSON should preserve document titles");
    expect_contains(section_process.stdout_text, "\"isLabel\": true",
                    "#2005: record-selected column-footer label section JSON should retain label identity");
    expect_contains(section_process.stdout_text, "\"selectedReportSectionAvailable\": true",
                    "#2005: record-selected column-footer label sections should advertise selected-section availability");
    expect_contains(section_process.stdout_text, "\"selectedReportSelectionAvailable\": true",
                    "#2005: record-selected column-footer label sections should advertise report-selection availability");
    expect_contains(section_process.stdout_text, "\"selectedReportSelectionKind\": \"section\"",
                    "#2005: record-selected column-footer label sections should expose section selection kind");
    expect_contains(section_process.stdout_text, "\"previewBoundsAvailable\": true",
                    "#2005: record-selected column-footer label section JSON should expose live preview availability");
    expect_contains(section_process.stdout_text, "\"previewBoundsLeft\": 0",
                    "#2005: record-selected column-footer label section JSON should preserve live preview left bounds");
    expect_contains(section_process.stdout_text, "\"previewBoundsTop\": 0",
                    "#2005: record-selected column-footer label section JSON should preserve live preview top bounds");
    expect_contains(section_process.stdout_text, "\"previewBoundsRight\": 0",
                    "#2005: record-selected column-footer label section JSON should preserve live preview right bounds");
    expect_contains(section_process.stdout_text, "\"previewBoundsBottom\": 3450",
                    "#2005: record-selected column-footer label section JSON should preserve live preview bottom bounds");
    expect_contains(section_process.stdout_text, "\"previewBoundsWidth\": 0",
                    "#2005: record-selected column-footer label section JSON should preserve live preview widths");
    expect_contains(section_process.stdout_text, "\"previewBoundsHeight\": 3450",
                    "#2005: record-selected column-footer label section JSON should preserve live preview heights");
    expect_contains(section_process.stdout_text, "\"deletedPreviewBoundsAvailable\": false",
                    "#2005: record-selected column-footer label section JSON should not fabricate deleted preview availability");
    expect_contains(section_process.stdout_text, "\"selectedReportObjectAvailable\": false",
                    "#2005: record-selected column-footer label sections should not advertise selected-object availability");
    expect_contains(section_process.stdout_text, "\"selectedReportObject\": null",
                    "#2005: record-selected column-footer label sections should serialize null selected objects");
    expect_contains(section_process.stdout_text, "\"selectedReportObjectSectionAvailable\": false",
                    "#2005: record-selected column-footer label sections should not advertise selected object-section availability");
    expect_contains(section_process.stdout_text, "\"selectedReportObjectSection\": null",
                    "#2005: record-selected column-footer label sections should serialize null selected object sections");
    expect_contains(section_process.stdout_text, "\"selectedReportSettingsAvailable\": false",
                    "#2005: record-selected column-footer label sections should not advertise selected-settings availability");
    expect_contains(section_process.stdout_text, "\"selectedReportSettings\": null",
                    "#2005: record-selected column-footer label sections should serialize null selected settings");
    expect_contains(section_process.stdout_text, "\"sectionCount\": 3",
                    "#2005: record-selected column-footer label section JSON should preserve live section counts");
    expect_contains(section_process.stdout_text, "\"deletedSectionCount\": 0",
                    "#2005: record-selected column-footer label section JSON should preserve deleted section counts");
    expect_contains_in_order(
        section_process.stdout_text,
        {
            "\"sections\": [",
            "\"bandKind\": \"column_header\"",
            "\"recordIndex\": 1",
            "\"bandKind\": \"detail\"",
            "\"recordIndex\": 2",
            "\"bandKind\": \"column_footer\"",
            "\"recordIndex\": 3"
        },
        "#2005: record-selected column-footer label section JSON should expose sibling section metadata");
    expect_contains_in_order(
        section_process.stdout_text,
        {
            "\"selectedReportSection\": {",
            "\"id\": \"column_footer_3\"",
            "\"bandKind\": \"column_footer\"",
            "\"recordIndex\": 3",
            "\"deleted\": false",
            "\"sectionIndex\": 2",
            "\"sectionCount\": 3",
            "\"top\": 3050",
            "\"height\": 400",
            "\"bottom\": 3450"
        },
        "#2005: record-selected column-footer label sections should expose selected section metadata");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_exposes_selected_deleted_column_footer_label_sections_by_record_selection(
    const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_selected_deleted_column_footer_label_sections_record_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path label_path = temp_root / "selected_deleted_column_footer_section_record.lbx";
    write_synthetic_report_table_for_stable_column_section_json(label_path);
    const auto delete_result = copperfin::vfp::set_record_deleted_flag(label_path.string(), 3U, true);
    expect(delete_result.ok && dbf_record_deleted(label_path, 3U),
           "#2007: record-selected deleted column-footer label fixture should mark the column-footer section deleted");

    const auto section_process = run_process_capture(
        studio_host_path,
        {"--path", label_path.string(), "--record", "3", "--json"},
        temp_root);

    if (section_process.exit_code != 0) {
        std::cerr << "studio host record-selected deleted column-footer label section stdout:\n"
                  << section_process.stdout_text << "\n";
        std::cerr << "studio host record-selected deleted column-footer label section stderr:\n"
                  << section_process.stderr_text << "\n";
        std::cerr << "fixture root: " << temp_root << "\n";
    }

    expect(section_process.exit_code == 0,
           "#2007: record-selected deleted column-footer label section JSON should exit successfully");
    expect_contains(section_process.stdout_text,
                    "\"documentTitle\": \"selected_deleted_column_footer_section_record.lbx\"",
                    "#2007: record-selected deleted column-footer label section JSON should preserve document titles");
    expect_contains(section_process.stdout_text, "\"isLabel\": true",
                    "#2007: record-selected deleted column-footer label section JSON should retain label identity");
    expect_contains(section_process.stdout_text, "\"selectedReportSectionAvailable\": true",
                    "#2007: record-selected deleted column-footer label sections should advertise selected-section availability");
    expect_contains(section_process.stdout_text, "\"selectedReportSelectionAvailable\": true",
                    "#2007: record-selected deleted column-footer label sections should advertise report-selection availability");
    expect_contains(section_process.stdout_text, "\"selectedReportSelectionKind\": \"section\"",
                    "#2007: record-selected deleted column-footer label sections should expose section selection kind");
    expect_contains(section_process.stdout_text, "\"previewBoundsAvailable\": true",
                    "#2007: record-selected deleted column-footer label section JSON should expose live preview availability");
    expect_contains(section_process.stdout_text, "\"previewBoundsLeft\": 0",
                    "#2007: record-selected deleted column-footer label section JSON should preserve live preview left bounds");
    expect_contains(section_process.stdout_text, "\"previewBoundsTop\": 0",
                    "#2007: record-selected deleted column-footer label section JSON should preserve live preview top bounds");
    expect_contains(section_process.stdout_text, "\"previewBoundsRight\": 0",
                    "#2007: record-selected deleted column-footer label section JSON should preserve live preview right bounds");
    expect_contains(section_process.stdout_text, "\"previewBoundsBottom\": 3050",
                    "#2007: record-selected deleted column-footer label section JSON should refresh live preview bottom bounds");
    expect_contains(section_process.stdout_text, "\"previewBoundsWidth\": 0",
                    "#2007: record-selected deleted column-footer label section JSON should preserve live preview widths");
    expect_contains(section_process.stdout_text, "\"previewBoundsHeight\": 3050",
                    "#2007: record-selected deleted column-footer label section JSON should refresh live preview heights");
    expect_contains(section_process.stdout_text, "\"deletedPreviewBoundsAvailable\": true",
                    "#2007: record-selected deleted column-footer label section JSON should expose deleted preview availability");
    expect_contains(section_process.stdout_text, "\"deletedPreviewBoundsLeft\": 0",
                    "#2007: record-selected deleted column-footer label section JSON should preserve deleted preview left bounds");
    expect_contains(section_process.stdout_text, "\"deletedPreviewBoundsTop\": 3050",
                    "#2007: record-selected deleted column-footer label section JSON should preserve deleted preview top bounds");
    expect_contains(section_process.stdout_text, "\"deletedPreviewBoundsRight\": 0",
                    "#2007: record-selected deleted column-footer label section JSON should preserve deleted preview right bounds");
    expect_contains(section_process.stdout_text, "\"deletedPreviewBoundsBottom\": 3450",
                    "#2007: record-selected deleted column-footer label section JSON should preserve deleted preview bottom bounds");
    expect_contains(section_process.stdout_text, "\"deletedPreviewBoundsWidth\": 0",
                    "#2007: record-selected deleted column-footer label section JSON should preserve deleted preview widths");
    expect_contains(section_process.stdout_text, "\"deletedPreviewBoundsHeight\": 400",
                    "#2007: record-selected deleted column-footer label section JSON should preserve deleted preview heights");
    expect_contains(section_process.stdout_text, "\"selectedReportObjectAvailable\": false",
                    "#2007: record-selected deleted column-footer label sections should not advertise selected-object availability");
    expect_contains(section_process.stdout_text, "\"selectedReportObject\": null",
                    "#2007: record-selected deleted column-footer label sections should serialize null selected objects");
    expect_contains(section_process.stdout_text, "\"selectedReportObjectSectionAvailable\": false",
                    "#2007: record-selected deleted column-footer label sections should not advertise selected object-section availability");
    expect_contains(section_process.stdout_text, "\"selectedReportObjectSection\": null",
                    "#2007: record-selected deleted column-footer label sections should serialize null selected object sections");
    expect_contains(section_process.stdout_text, "\"selectedReportSettingsAvailable\": false",
                    "#2007: record-selected deleted column-footer label sections should not advertise selected-settings availability");
    expect_contains(section_process.stdout_text, "\"selectedReportSettings\": null",
                    "#2007: record-selected deleted column-footer label sections should serialize null selected settings");
    expect_contains(section_process.stdout_text, "\"sectionCount\": 2",
                    "#2007: record-selected deleted column-footer label section JSON should preserve live section counts");
    expect_contains(section_process.stdout_text, "\"deletedSectionCount\": 1",
                    "#2007: record-selected deleted column-footer label section JSON should preserve deleted section counts");
    expect_contains_in_order(
        section_process.stdout_text,
        {
            "\"deletedSections\": [",
            "\"id\": \"column_footer_3\"",
            "\"bandKind\": \"column_footer\"",
            "\"recordIndex\": 3",
            "\"deleted\": true"
        },
        "#2007: record-selected deleted column-footer label section JSON should expose deleted column-footer metadata");
    expect_contains_in_order(
        section_process.stdout_text,
        {
            "\"sections\": [",
            "\"bandKind\": \"column_header\"",
            "\"recordIndex\": 1",
            "\"bandKind\": \"detail\"",
            "\"recordIndex\": 2"
        },
        "#2007: record-selected deleted column-footer label section JSON should expose live sibling section metadata");
    expect_contains_in_order(
        section_process.stdout_text,
        {
            "\"selectedReportSection\": {",
            "\"id\": \"column_footer_3\"",
            "\"bandKind\": \"column_footer\"",
            "\"recordIndex\": 3",
            "\"deleted\": true",
            "\"sectionIndex\": null",
            "\"sectionCount\": 0",
            "\"top\": 3050",
            "\"height\": 400",
            "\"bottom\": 3450"
        },
        "#2007: record-selected deleted column-footer label sections should expose selected deleted column-footer metadata");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_exposes_selected_page_header_label_sections_by_record_selection(
    const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_selected_page_header_label_sections_record_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path label_path = temp_root / "selected_page_header_section_record.lbx";
    write_synthetic_report_table_for_stable_page_header_section_json(label_path);

    const auto section_process = run_process_capture(
        studio_host_path,
        {"--path", label_path.string(), "--record", "1", "--json"},
        temp_root);

    if (section_process.exit_code != 0) {
        std::cerr << "studio host record-selected page-header label section stdout:\n"
                  << section_process.stdout_text << "\n";
        std::cerr << "studio host record-selected page-header label section stderr:\n"
                  << section_process.stderr_text << "\n";
        std::cerr << "fixture root: " << temp_root << "\n";
    }

    expect(section_process.exit_code == 0,
           "#2009: record-selected page-header label section JSON should exit successfully");
    expect_contains(section_process.stdout_text,
                    "\"documentTitle\": \"selected_page_header_section_record.lbx\"",
                    "#2009: record-selected page-header label section JSON should preserve document titles");
    expect_contains(section_process.stdout_text, "\"isLabel\": true",
                    "#2009: record-selected page-header label section JSON should retain label identity");
    expect_contains(section_process.stdout_text, "\"selectedReportSectionAvailable\": true",
                    "#2009: record-selected page-header label sections should advertise selected-section availability");
    expect_contains(section_process.stdout_text, "\"selectedReportSelectionAvailable\": true",
                    "#2009: record-selected page-header label sections should advertise report-selection availability");
    expect_contains(section_process.stdout_text, "\"selectedReportSelectionKind\": \"section\"",
                    "#2009: record-selected page-header label sections should expose section selection kind");
    expect_contains(section_process.stdout_text, "\"previewBoundsAvailable\": true",
                    "#2009: record-selected page-header label section JSON should expose live preview availability");
    expect_contains(section_process.stdout_text, "\"previewBoundsLeft\": 0",
                    "#2009: record-selected page-header label section JSON should preserve live preview left bounds");
    expect_contains(section_process.stdout_text, "\"previewBoundsTop\": 0",
                    "#2009: record-selected page-header label section JSON should preserve live preview top bounds");
    expect_contains(section_process.stdout_text, "\"previewBoundsRight\": 0",
                    "#2009: record-selected page-header label section JSON should preserve live preview right bounds");
    expect_contains(section_process.stdout_text, "\"previewBoundsBottom\": 3700",
                    "#2009: record-selected page-header label section JSON should preserve live preview bottom bounds");
    expect_contains(section_process.stdout_text, "\"previewBoundsWidth\": 0",
                    "#2009: record-selected page-header label section JSON should preserve live preview widths");
    expect_contains(section_process.stdout_text, "\"previewBoundsHeight\": 3700",
                    "#2009: record-selected page-header label section JSON should preserve live preview heights");
    expect_contains(section_process.stdout_text, "\"deletedPreviewBoundsAvailable\": false",
                    "#2009: record-selected page-header label section JSON should not fabricate deleted preview availability");
    expect_contains(section_process.stdout_text, "\"selectedReportObjectAvailable\": false",
                    "#2009: record-selected page-header label sections should not advertise selected-object availability");
    expect_contains(section_process.stdout_text, "\"selectedReportObject\": null",
                    "#2009: record-selected page-header label sections should serialize null selected objects");
    expect_contains(section_process.stdout_text, "\"selectedReportObjectSectionAvailable\": false",
                    "#2009: record-selected page-header label sections should not advertise selected object-section availability");
    expect_contains(section_process.stdout_text, "\"selectedReportObjectSection\": null",
                    "#2009: record-selected page-header label sections should serialize null selected object sections");
    expect_contains(section_process.stdout_text, "\"selectedReportSettingsAvailable\": false",
                    "#2009: record-selected page-header label sections should not advertise selected-settings availability");
    expect_contains(section_process.stdout_text, "\"selectedReportSettings\": null",
                    "#2009: record-selected page-header label sections should serialize null selected settings");
    expect_contains(section_process.stdout_text, "\"sectionCount\": 3",
                    "#2009: record-selected page-header label section JSON should preserve live section counts");
    expect_contains(section_process.stdout_text, "\"deletedSectionCount\": 0",
                    "#2009: record-selected page-header label section JSON should preserve deleted section counts");
    expect_contains_in_order(
        section_process.stdout_text,
        {
            "\"sections\": [",
            "\"bandKind\": \"page_header\"",
            "\"recordIndex\": 1",
            "\"bandKind\": \"detail\"",
            "\"recordIndex\": 2",
            "\"bandKind\": \"page_footer\"",
            "\"recordIndex\": 3"
        },
        "#2009: record-selected page-header label section JSON should expose sibling section metadata");
    expect_contains_in_order(
        section_process.stdout_text,
        {
            "\"selectedReportSection\": {",
            "\"id\": \"page_header_1\"",
            "\"bandKind\": \"page_header\"",
            "\"recordIndex\": 1",
            "\"deleted\": false",
            "\"sectionIndex\": 0",
            "\"sectionCount\": 3",
            "\"top\": 0",
            "\"height\": 700",
            "\"bottom\": 700"
        },
        "#2009: record-selected page-header label sections should expose selected section metadata");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_exposes_selected_deleted_page_header_label_sections_by_record_selection(
    const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_selected_deleted_page_header_label_sections_record_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path label_path = temp_root / "selected_deleted_page_header_section_record.lbx";
    write_synthetic_report_table_for_stable_page_header_section_json(label_path);
    const auto delete_result = copperfin::vfp::set_record_deleted_flag(label_path.string(), 1U, true);
    expect(delete_result.ok && dbf_record_deleted(label_path, 1U),
           "#2011: record-selected deleted page-header label fixture should mark the page-header section deleted");

    const auto section_process = run_process_capture(
        studio_host_path,
        {"--path", label_path.string(), "--record", "1", "--json"},
        temp_root);

    if (section_process.exit_code != 0) {
        std::cerr << "studio host record-selected deleted page-header label section stdout:\n"
                  << section_process.stdout_text << "\n";
        std::cerr << "studio host record-selected deleted page-header label section stderr:\n"
                  << section_process.stderr_text << "\n";
        std::cerr << "fixture root: " << temp_root << "\n";
    }

    expect(section_process.exit_code == 0,
           "#2011: record-selected deleted page-header label section JSON should exit successfully");
    expect_contains(section_process.stdout_text,
                    "\"documentTitle\": \"selected_deleted_page_header_section_record.lbx\"",
                    "#2011: record-selected deleted page-header label section JSON should preserve document titles");
    expect_contains(section_process.stdout_text, "\"isLabel\": true",
                    "#2011: record-selected deleted page-header label section JSON should retain label identity");
    expect_contains(section_process.stdout_text, "\"selectedReportSectionAvailable\": true",
                    "#2011: record-selected deleted page-header label sections should advertise selected-section availability");
    expect_contains(section_process.stdout_text, "\"selectedReportSelectionAvailable\": true",
                    "#2011: record-selected deleted page-header label sections should advertise report-selection availability");
    expect_contains(section_process.stdout_text, "\"selectedReportSelectionKind\": \"section\"",
                    "#2011: record-selected deleted page-header label sections should expose section selection kind");
    expect_contains(section_process.stdout_text, "\"previewBoundsAvailable\": true",
                    "#2011: record-selected deleted page-header label section JSON should expose live preview availability");
    expect_contains(section_process.stdout_text, "\"previewBoundsLeft\": 0",
                    "#2011: record-selected deleted page-header label section JSON should preserve live preview left bounds");
    expect_contains(section_process.stdout_text, "\"previewBoundsTop\": 700",
                    "#2011: record-selected deleted page-header label section JSON should refresh live preview top bounds");
    expect_contains(section_process.stdout_text, "\"previewBoundsRight\": 0",
                    "#2011: record-selected deleted page-header label section JSON should preserve live preview right bounds");
    expect_contains(section_process.stdout_text, "\"previewBoundsBottom\": 3700",
                    "#2011: record-selected deleted page-header label section JSON should preserve live preview bottom bounds");
    expect_contains(section_process.stdout_text, "\"previewBoundsWidth\": 0",
                    "#2011: record-selected deleted page-header label section JSON should preserve live preview widths");
    expect_contains(section_process.stdout_text, "\"previewBoundsHeight\": 3000",
                    "#2011: record-selected deleted page-header label section JSON should refresh live preview heights");
    expect_contains(section_process.stdout_text, "\"deletedPreviewBoundsAvailable\": true",
                    "#2011: record-selected deleted page-header label section JSON should expose deleted preview availability");
    expect_contains(section_process.stdout_text, "\"deletedPreviewBoundsLeft\": 0",
                    "#2011: record-selected deleted page-header label section JSON should preserve deleted preview left bounds");
    expect_contains(section_process.stdout_text, "\"deletedPreviewBoundsTop\": 0",
                    "#2011: record-selected deleted page-header label section JSON should preserve deleted preview top bounds");
    expect_contains(section_process.stdout_text, "\"deletedPreviewBoundsRight\": 0",
                    "#2011: record-selected deleted page-header label section JSON should preserve deleted preview right bounds");
    expect_contains(section_process.stdout_text, "\"deletedPreviewBoundsBottom\": 700",
                    "#2011: record-selected deleted page-header label section JSON should preserve deleted preview bottom bounds");
    expect_contains(section_process.stdout_text, "\"deletedPreviewBoundsWidth\": 0",
                    "#2011: record-selected deleted page-header label section JSON should preserve deleted preview widths");
    expect_contains(section_process.stdout_text, "\"deletedPreviewBoundsHeight\": 700",
                    "#2011: record-selected deleted page-header label section JSON should preserve deleted preview heights");
    expect_contains(section_process.stdout_text, "\"selectedReportObjectAvailable\": false",
                    "#2011: record-selected deleted page-header label sections should not advertise selected-object availability");
    expect_contains(section_process.stdout_text, "\"selectedReportObject\": null",
                    "#2011: record-selected deleted page-header label sections should serialize null selected objects");
    expect_contains(section_process.stdout_text, "\"selectedReportObjectSectionAvailable\": false",
                    "#2011: record-selected deleted page-header label sections should not advertise selected object-section availability");
    expect_contains(section_process.stdout_text, "\"selectedReportObjectSection\": null",
                    "#2011: record-selected deleted page-header label sections should serialize null selected object sections");
    expect_contains(section_process.stdout_text, "\"selectedReportSettingsAvailable\": false",
                    "#2011: record-selected deleted page-header label sections should not advertise selected-settings availability");
    expect_contains(section_process.stdout_text, "\"selectedReportSettings\": null",
                    "#2011: record-selected deleted page-header label sections should serialize null selected settings");
    expect_contains(section_process.stdout_text, "\"sectionCount\": 2",
                    "#2011: record-selected deleted page-header label section JSON should preserve live section counts");
    expect_contains(section_process.stdout_text, "\"deletedSectionCount\": 1",
                    "#2011: record-selected deleted page-header label section JSON should preserve deleted section counts");
    expect_contains_in_order(
        section_process.stdout_text,
        {
            "\"deletedSections\": [",
            "\"id\": \"page_header_1\"",
            "\"bandKind\": \"page_header\"",
            "\"recordIndex\": 1",
            "\"deleted\": true"
        },
        "#2011: record-selected deleted page-header label section JSON should expose deleted page-header metadata");
    expect_contains_in_order(
        section_process.stdout_text,
        {
            "\"sections\": [",
            "\"bandKind\": \"detail\"",
            "\"recordIndex\": 2",
            "\"bandKind\": \"page_footer\"",
            "\"recordIndex\": 3"
        },
        "#2011: record-selected deleted page-header label section JSON should expose live sibling section metadata");
    expect_contains_in_order(
        section_process.stdout_text,
        {
            "\"selectedReportSection\": {",
            "\"id\": \"page_header_1\"",
            "\"bandKind\": \"page_header\"",
            "\"recordIndex\": 1",
            "\"deleted\": true",
            "\"sectionIndex\": null",
            "\"sectionCount\": 0",
            "\"top\": 0",
            "\"height\": 700",
            "\"bottom\": 700"
        },
        "#2011: record-selected deleted page-header label sections should expose selected deleted page-header metadata");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_exposes_label_layout_parity(const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_label_layout_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path label_path = temp_root / "mailing.lbx";
    write_synthetic_report_table_for_layout_json(label_path);

    const auto summary_process = run_process_capture(
        studio_host_path,
        {"--path", label_path.string(), "--json"},
        temp_root);

    if (summary_process.exit_code != 0) {
        std::cerr << "studio host label layout summary stdout:\n"
                  << summary_process.stdout_text << "\n";
        std::cerr << "studio host label layout summary stderr:\n"
                  << summary_process.stderr_text << "\n";
        std::cerr << "fixture root: " << temp_root << "\n";
    }

    expect(summary_process.exit_code == 0,
           "#1501: unselected label layout summary JSON should exit successfully");
    expect_contains(summary_process.stdout_text, "\"reportLayout\": {",
                    "#1501: unselected label documents should expose report-layout JSON");
    expect_contains(summary_process.stdout_text, "\"isLabel\": true",
                    "#1501: unselected label layout JSON should retain label identity");
    expect_contains(summary_process.stdout_text, "\"documentTitle\": \"mailing.lbx\"",
                    "#1501: unselected label layout JSON should preserve label document titles");
    expect_contains(summary_process.stdout_text, "\"previewBoundsAvailable\": true",
                    "#1516: label layout JSON should expose preview bounds availability");
    expect_contains(summary_process.stdout_text, "\"previewBoundsRight\": 5200",
                    "#1516: label layout JSON should expose shared right preview bounds");
    expect_contains(summary_process.stdout_text, "\"previewBoundsBottom\": 8100",
                    "#1516: label layout JSON should expose shared bottom preview bounds");
    expect_contains(summary_process.stdout_text, "\"deletedPreviewBoundsAvailable\": true",
                    "#1524: label layout JSON should expose deleted preview bounds availability");
    expect_contains(summary_process.stdout_text, "\"deletedPreviewBoundsLeft\": 1000",
                    "#1524: label layout JSON should expose deleted preview left bounds");
    expect_contains(summary_process.stdout_text, "\"deletedPreviewBoundsTop\": 2600",
                    "#1524: label layout JSON should expose deleted preview top bounds");
    expect_contains(summary_process.stdout_text, "\"deletedPreviewBoundsRight\": 2200",
                    "#1524: label layout JSON should expose deleted preview right bounds");
    expect_contains(summary_process.stdout_text, "\"deletedPreviewBoundsBottom\": 2900",
                    "#1524: label layout JSON should expose deleted preview bottom bounds");
    expect_contains(summary_process.stdout_text, "\"deletedPreviewBoundsWidth\": 1200",
                    "#1524: label layout JSON should expose deleted preview width");
    expect_contains(summary_process.stdout_text, "\"deletedPreviewBoundsHeight\": 300",
                    "#1524: label layout JSON should expose deleted preview height");
    expect_contains(summary_process.stdout_text, "\"liveObjectCount\": 3",
                    "#1516: label layout JSON should summarize live placed and unplaced object counts");
    expect_contains(summary_process.stdout_text, "\"placedObjectCount\": 2",
                    "#1522: label layout JSON should summarize section-contained live object counts");
    expect_contains(summary_process.stdout_text, "\"deletedPlacedObjectCount\": 1",
                    "#1523: label layout JSON should summarize deleted objects still inside section bands");
    expect_contains(summary_process.stdout_text, "\"deletedUnplacedObjectCount\": 0",
                    "#1523: label layout JSON should not fabricate deleted unplaced object counts");
    expect_contains(summary_process.stdout_text, "\"objectKindCount\": 3",
                    "#1519: label layout JSON should summarize live object-kind count buckets");
    expect_contains(summary_process.stdout_text, "\"objectKindCounts\": [",
                    "#1519: label layout JSON should expose live object-kind count summaries");
    expect_contains(summary_process.stdout_text, "{\"kind\": \"field\", \"count\": 1}",
                    "#1519: label layout JSON should count live field objects");
    expect_contains(summary_process.stdout_text, "{\"kind\": \"label\", \"count\": 1}",
                    "#1519: label layout JSON should count live label objects");
    expect_contains(summary_process.stdout_text, "{\"kind\": \"line\", \"count\": 1}",
                    "#1519: label layout JSON should count live unplaced line objects");
    expect_contains(summary_process.stdout_text, "\"unplacedObjectKindCount\": 1",
                    "#1519: label layout JSON should summarize unplaced object-kind count buckets");
    expect_contains(summary_process.stdout_text, "\"unplacedObjectKindCounts\": [\n        {\"kind\": \"line\", \"count\": 1}\n      ]",
                    "#1519: label layout JSON should count unplaced line objects");
    expect_contains(summary_process.stdout_text, "\"deletedObjectKindCount\": 1",
                    "#1519: label layout JSON should summarize deleted object-kind count buckets");
    expect_contains(summary_process.stdout_text, "\"deletedObjectKindCounts\": [\n        {\"kind\": \"label\", \"count\": 1}\n      ]",
                    "#1519: label layout JSON should count deleted label objects");
    expect_contains(summary_process.stdout_text, "\"sectionKindCount\": 2",
                    "#1520: label layout JSON should summarize live section band-kind buckets");
    expect_contains(summary_process.stdout_text, "\"sectionKindCounts\": [\n        {\"kind\": \"detail\", \"count\": 1},\n        {\"kind\": \"page_header\", \"count\": 1}\n      ]",
                    "#1520: label layout JSON should count live detail and page-header sections");
    expect_contains(summary_process.stdout_text, "\"deletedSectionKindCount\": 0",
                    "#1520: label layout JSON should not fabricate deleted section band-kind buckets");
    expect_contains(summary_process.stdout_text, "\"sectionHeightTotal\": 7000",
                    "#1521: label layout JSON should summarize live section heights");
    expect_contains(summary_process.stdout_text, "\"deletedSectionHeightTotal\": 0",
                    "#1521: label layout JSON should not fabricate deleted section heights");
    expect_contains(summary_process.stdout_text, "\"pageSetupAvailable\": true",
                    "#1517: label layout JSON should expose page setup summary availability");
    expect_contains(summary_process.stdout_text, "\"orientationCode\": 0",
                    "#1517: label layout JSON should expose orientation codes");
    expect_contains(summary_process.stdout_text, "\"paperSizeCode\": 1",
                    "#1517: label layout JSON should expose paper-size codes");
    expect_contains(summary_process.stdout_text, "\"topMargin\": 10",
                    "#1517: label layout JSON should expose top margins");
    expect_contains(summary_process.stdout_text, "\"bottomMargin\": 20",
                    "#1517: label layout JSON should expose bottom margins");
    expect_contains(summary_process.stdout_text, "\"gridVertical\": 4",
                    "#1517: label layout JSON should expose vertical grid spacing");
    expect_contains(summary_process.stdout_text, "\"gridHorizontal\": 8",
                    "#1517: label layout JSON should expose horizontal grid spacing");
    expect_contains(summary_process.stdout_text, "\"settingCount\": 6",
                    "#1501: unselected label layout JSON should summarize live settings");
    expect_contains(summary_process.stdout_text, "\"sectionCount\": 2",
                    "#1501: unselected label layout JSON should summarize live sections");
    expect_contains(summary_process.stdout_text, "\"deletedObjectCount\": 1",
                    "#1501: unselected label layout JSON should summarize deleted objects");
    expect_contains(summary_process.stdout_text, "\"unplacedObjectCount\": 1",
                    "#1501: unselected label layout JSON should summarize unplaced objects");
    expect_contains(summary_process.stdout_text, "\"name\": \"ORIENTATION\", \"recordIndex\": 0, \"fieldIndex\": 2, \"sourceLineIndex\": 0, \"memoBlockNumber\": 1, \"value\": \"0\"",
                    "#1501: unselected label layout JSON should expose memo-line setting provenance");
    expect_contains(summary_process.stdout_text, "\"title\": \"Record 5\"",
                    "#1501: unselected label layout JSON should preserve synthesized unplaced-object titles");
    expect_contains(summary_process.stdout_text, "\"selectedReportSelectionAvailable\": false",
                    "#1501: unselected label layout JSON should not fabricate report-selection availability");
    expect_contains(summary_process.stdout_text, "\"selectedReportSelectionKind\": \"none\"",
                    "#1501: unselected label layout JSON should expose the none report-selection kind");

    const auto object_process = run_process_capture(
        studio_host_path,
        {"--path", label_path.string(), "--record", "3", "--json"},
        temp_root);

    if (object_process.exit_code != 0) {
        std::cerr << "studio host label layout object stdout:\n" << object_process.stdout_text << "\n";
        std::cerr << "studio host label layout object stderr:\n" << object_process.stderr_text << "\n";
        std::cerr << "fixture root: " << temp_root << "\n";
    }

    expect(object_process.exit_code == 0,
           "#1481: label layout JSON should exit successfully for selected label objects");
    expect_contains(object_process.stdout_text, "\"reportLayout\": {",
                    "#1481: label documents should expose report-layout JSON");
    expect_contains(object_process.stdout_text, "\"isLabel\": true",
                    "#1481: label layout JSON should identify label assets");
    expect_contains(object_process.stdout_text, "\"documentTitle\": \"mailing.lbx\"",
                    "#1481: label layout JSON should preserve label document titles");
    expect_contains(object_process.stdout_text, "\"settingCount\": 6",
                    "#1481: label layout JSON should expose live settings");
    expect_contains(object_process.stdout_text, "\"sectionCount\": 2",
                    "#1481: label layout JSON should expose live sections");
    expect_contains(object_process.stdout_text, "\"selectedReportSelectionKind\": \"object\"",
                    "#1481: selected label objects should reuse report object selection kind");
    expect_contains(object_process.stdout_text, "\"selectedReportObjectAvailable\": true",
                    "#1481: selected label objects should expose selected report-object JSON");
    expect_contains(object_process.stdout_text, "\"previewBoundsAvailable\": true",
                    "#1966: selected label object JSON should expose live preview availability");
    expect_contains(object_process.stdout_text, "\"previewBoundsLeft\": 0",
                    "#1966: selected label object JSON should preserve live preview left bounds");
    expect_contains(object_process.stdout_text, "\"previewBoundsTop\": 0",
                    "#1966: selected label object JSON should preserve live preview top bounds");
    expect_contains(object_process.stdout_text, "\"previewBoundsRight\": 5200",
                    "#1966: selected label object JSON should preserve live preview right bounds");
    expect_contains(object_process.stdout_text, "\"previewBoundsBottom\": 8100",
                    "#1966: selected label object JSON should preserve live preview bottom bounds");
    expect_contains(object_process.stdout_text, "\"previewBoundsWidth\": 5200",
                    "#1966: selected label object JSON should preserve live preview widths");
    expect_contains(object_process.stdout_text, "\"previewBoundsHeight\": 8100",
                    "#1966: selected label object JSON should preserve live preview heights");
    expect_contains(object_process.stdout_text, "\"deletedPreviewBoundsAvailable\": true",
                    "#1966: selected label object JSON should expose deleted preview availability");
    expect_contains(object_process.stdout_text, "\"deletedPreviewBoundsLeft\": 1000",
                    "#1966: selected label object JSON should preserve deleted preview left bounds");
    expect_contains(object_process.stdout_text, "\"deletedPreviewBoundsTop\": 2600",
                    "#1966: selected label object JSON should preserve deleted preview top bounds");
    expect_contains(object_process.stdout_text, "\"deletedPreviewBoundsRight\": 2200",
                    "#1966: selected label object JSON should preserve deleted preview right bounds");
    expect_contains(object_process.stdout_text, "\"deletedPreviewBoundsBottom\": 2900",
                    "#1966: selected label object JSON should preserve deleted preview bottom bounds");
    expect_contains(object_process.stdout_text, "\"deletedPreviewBoundsWidth\": 1200",
                    "#1966: selected label object JSON should preserve deleted preview widths");
    expect_contains(object_process.stdout_text, "\"deletedPreviewBoundsHeight\": 300",
                    "#1966: selected label object JSON should preserve deleted preview heights");
    expect_contains(object_process.stdout_text, "\"selectedReportSectionAvailable\": false",
                    "#1506: selected label objects should not advertise selected-section availability");
    expect_contains(object_process.stdout_text, "\"selectedReportSection\": null",
                    "#1506: selected label objects should serialize null selected sections");
    expect_contains(object_process.stdout_text, "\"selectedReportSettingsAvailable\": false",
                    "#1506: selected label objects should not advertise selected-settings availability");
    expect_contains(object_process.stdout_text, "\"selectedReportSettings\": null",
                    "#1506: selected label objects should serialize null selected settings");
    expect_contains(object_process.stdout_text, "\"objectKind\": \"field\"",
                    "#1481: selected label objects should preserve report object kinds");
    expect_contains(object_process.stdout_text, "\"containingSectionId\": \"detail_2\"",
                    "#1481: selected label objects should expose containing section ids");
    expect_contains(object_process.stdout_text, "\"selectedReportObjectSectionAvailable\": true",
                    "#1481: selected label objects should expose containing section availability");

    const auto page_header_object_process = run_process_capture(
        studio_host_path,
        {"--path", label_path.string(), "--record", "4", "--json"},
        temp_root);

    if (page_header_object_process.exit_code != 0) {
        std::cerr << "studio host selected page-header label object stdout:\n"
                  << page_header_object_process.stdout_text << "\n";
        std::cerr << "studio host selected page-header label object stderr:\n"
                  << page_header_object_process.stderr_text << "\n";
        std::cerr << "fixture root: " << temp_root << "\n";
    }

    expect(page_header_object_process.exit_code == 0,
           "#1973: selected page-header label object JSON should exit successfully");
    expect_contains(page_header_object_process.stdout_text, "\"isLabel\": true",
                    "#1973: selected page-header label object JSON should retain label identity");
    expect_contains(page_header_object_process.stdout_text, "\"documentTitle\": \"mailing.lbx\"",
                    "#1973: selected page-header label object JSON should preserve label document titles");
    expect_contains(page_header_object_process.stdout_text, "\"selectedReportObjectAvailable\": true",
                    "#1973: page-header label object selections should advertise selected-object availability");
    expect_contains(page_header_object_process.stdout_text, "\"selectedReportSelectionAvailable\": true",
                    "#1973: page-header label object selections should advertise report-selection availability");
    expect_contains(page_header_object_process.stdout_text, "\"selectedReportSelectionKind\": \"object\"",
                    "#1973: page-header label object selections should expose object selection kind");
    expect_contains(page_header_object_process.stdout_text, "\"previewBoundsAvailable\": true",
                    "#1973: selected page-header label object JSON should expose live preview availability");
    expect_contains(page_header_object_process.stdout_text, "\"previewBoundsLeft\": 0",
                    "#1973: selected page-header label object JSON should preserve live preview left bounds");
    expect_contains(page_header_object_process.stdout_text, "\"previewBoundsTop\": 0",
                    "#1973: selected page-header label object JSON should preserve live preview top bounds");
    expect_contains(page_header_object_process.stdout_text, "\"previewBoundsRight\": 5200",
                    "#1973: selected page-header label object JSON should preserve live preview right bounds");
    expect_contains(page_header_object_process.stdout_text, "\"previewBoundsBottom\": 8100",
                    "#1973: selected page-header label object JSON should preserve live preview bottom bounds");
    expect_contains(page_header_object_process.stdout_text, "\"previewBoundsWidth\": 5200",
                    "#1973: selected page-header label object JSON should preserve live preview widths");
    expect_contains(page_header_object_process.stdout_text, "\"previewBoundsHeight\": 8100",
                    "#1973: selected page-header label object JSON should preserve live preview heights");
    expect_contains(page_header_object_process.stdout_text, "\"deletedPreviewBoundsAvailable\": true",
                    "#1973: selected page-header label object JSON should expose deleted preview availability");
    expect_contains(page_header_object_process.stdout_text, "\"deletedPreviewBoundsLeft\": 1000",
                    "#1973: selected page-header label object JSON should preserve deleted preview left bounds");
    expect_contains(page_header_object_process.stdout_text, "\"deletedPreviewBoundsTop\": 2600",
                    "#1973: selected page-header label object JSON should preserve deleted preview top bounds");
    expect_contains(page_header_object_process.stdout_text, "\"deletedPreviewBoundsRight\": 2200",
                    "#1973: selected page-header label object JSON should preserve deleted preview right bounds");
    expect_contains(page_header_object_process.stdout_text, "\"deletedPreviewBoundsBottom\": 2900",
                    "#1973: selected page-header label object JSON should preserve deleted preview bottom bounds");
    expect_contains(page_header_object_process.stdout_text, "\"deletedPreviewBoundsWidth\": 1200",
                    "#1973: selected page-header label object JSON should preserve deleted preview widths");
    expect_contains(page_header_object_process.stdout_text, "\"deletedPreviewBoundsHeight\": 300",
                    "#1973: selected page-header label object JSON should preserve deleted preview heights");
    expect_contains(page_header_object_process.stdout_text, "\"selectedReportSectionAvailable\": false",
                    "#1973: selected page-header label objects should not advertise selected-section availability");
    expect_contains(page_header_object_process.stdout_text, "\"selectedReportSection\": null",
                    "#1973: selected page-header label objects should serialize null selected sections");
    expect_contains(page_header_object_process.stdout_text, "\"selectedReportSettingsAvailable\": false",
                    "#1973: selected page-header label objects should not advertise selected-settings availability");
    expect_contains(page_header_object_process.stdout_text, "\"selectedReportSettings\": null",
                    "#1973: selected page-header label objects should serialize null selected settings");
    expect_contains_in_order(
        page_header_object_process.stdout_text,
        {
            "\"selectedReportObject\": {",
            "\"recordIndex\": 4",
            "\"deleted\": false",
            "\"containingSectionId\": \"page_header_1\"",
            "\"containingSectionRecordIndex\": 1",
            "\"sectionRelativeTop\": 100",
            "\"sectionRelativeBottom\": 450",
            "\"sectionObjectIndex\": 0",
            "\"sectionObjectCount\": 1",
            "\"objectTypeCode\": 5",
            "\"objectKind\": \"label\"",
            "\"expression\": \"\\\"Invoice\\\"\""
        },
        "#1973: page-header label object selections should expose selected-object metadata");
    expect_contains(page_header_object_process.stdout_text, "\"right\": 2700",
                    "#1973: selected page-header label object JSON should expose object right-edge coordinates");
    expect_contains(page_header_object_process.stdout_text, "\"bottom\": 450",
                    "#1973: selected page-header label object JSON should expose object bottom-edge coordinates");
    expect_contains(page_header_object_process.stdout_text, "\"selectedReportObjectSectionAvailable\": true",
                    "#1973: page-header label objects should advertise containing-section availability");
    expect_contains(page_header_object_process.stdout_text, "\"selectedReportObjectSection\": {",
                    "#1973: page-header label objects should expose containing-section JSON");
    expect_contains(page_header_object_process.stdout_text, "\"id\": \"page_header_1\"",
                    "#1973: containing-section JSON should expose selected page-header label object section ids");
    expect_contains(page_header_object_process.stdout_text, "\"bandKind\": \"page_header\"",
                    "#1973: containing-section JSON should expose selected page-header label object band kinds");

    const fs::path deleted_page_header_object_path = temp_root / "deleted_page_header_label_object.lbx";
    write_synthetic_report_table_for_layout_json(deleted_page_header_object_path);
    const auto delete_page_header_object_result =
        copperfin::vfp::set_record_deleted_flag(deleted_page_header_object_path.string(), 4U, true);
    expect(delete_page_header_object_result.ok,
           "#1975: selected deleted page-header label object fixture should mark the page-header object deleted");

    const auto deleted_page_header_object_process = run_process_capture(
        studio_host_path,
        {"--path", deleted_page_header_object_path.string(), "--record", "4", "--json"},
        temp_root);

    if (deleted_page_header_object_process.exit_code != 0) {
        std::cerr << "studio host selected deleted page-header label object stdout:\n"
                  << deleted_page_header_object_process.stdout_text << "\n";
        std::cerr << "studio host selected deleted page-header label object stderr:\n"
                  << deleted_page_header_object_process.stderr_text << "\n";
        std::cerr << "fixture root: " << temp_root << "\n";
    }

    expect(deleted_page_header_object_process.exit_code == 0,
           "#1975: selected deleted page-header label object JSON should exit successfully");
    expect_contains(deleted_page_header_object_process.stdout_text, "\"isLabel\": true",
                    "#1975: selected deleted page-header label object JSON should retain label identity");
    expect_contains(deleted_page_header_object_process.stdout_text, "\"documentTitle\": \"deleted_page_header_label_object.lbx\"",
                    "#1975: selected deleted page-header label object JSON should preserve label document titles");
    expect_contains(deleted_page_header_object_process.stdout_text, "\"selectedReportObjectAvailable\": true",
                    "#1975: deleted page-header label object selections should advertise selected-object availability");
    expect_contains(deleted_page_header_object_process.stdout_text, "\"selectedReportSelectionAvailable\": true",
                    "#1975: deleted page-header label object selections should advertise report-selection availability");
    expect_contains(deleted_page_header_object_process.stdout_text, "\"selectedReportSelectionKind\": \"object\"",
                    "#1975: deleted page-header label object selections should expose object selection kind");
    expect_contains(deleted_page_header_object_process.stdout_text, "\"previewBoundsAvailable\": true",
                    "#1975: selected deleted page-header label object JSON should preserve live preview availability");
    expect_contains(deleted_page_header_object_process.stdout_text, "\"previewBoundsLeft\": 0",
                    "#1975: selected deleted page-header label object JSON should preserve live preview left bounds");
    expect_contains(deleted_page_header_object_process.stdout_text, "\"previewBoundsTop\": 0",
                    "#1975: selected deleted page-header label object JSON should preserve live preview top bounds");
    expect_contains(deleted_page_header_object_process.stdout_text, "\"previewBoundsRight\": 5200",
                    "#1975: selected deleted page-header label object JSON should preserve live preview right bounds");
    expect_contains(deleted_page_header_object_process.stdout_text, "\"previewBoundsBottom\": 8100",
                    "#1975: selected deleted page-header label object JSON should preserve live preview bottom bounds");
    expect_contains(deleted_page_header_object_process.stdout_text, "\"previewBoundsWidth\": 5200",
                    "#1975: selected deleted page-header label object JSON should preserve live preview widths");
    expect_contains(deleted_page_header_object_process.stdout_text, "\"previewBoundsHeight\": 8100",
                    "#1975: selected deleted page-header label object JSON should preserve live preview heights");
    expect_contains(deleted_page_header_object_process.stdout_text, "\"deletedPreviewBoundsAvailable\": true",
                    "#1975: selected deleted page-header label object JSON should expose deleted preview availability");
    expect_contains(deleted_page_header_object_process.stdout_text, "\"deletedPreviewBoundsLeft\": 900",
                    "#1975: selected deleted page-header label object JSON should expand deleted preview left bounds");
    expect_contains(deleted_page_header_object_process.stdout_text, "\"deletedPreviewBoundsTop\": 100",
                    "#1975: selected deleted page-header label object JSON should expand deleted preview top bounds");
    expect_contains(deleted_page_header_object_process.stdout_text, "\"deletedPreviewBoundsRight\": 2700",
                    "#1975: selected deleted page-header label object JSON should expand deleted preview right bounds");
    expect_contains(deleted_page_header_object_process.stdout_text, "\"deletedPreviewBoundsBottom\": 2900",
                    "#1975: selected deleted page-header label object JSON should preserve deleted preview bottom bounds");
    expect_contains(deleted_page_header_object_process.stdout_text, "\"deletedPreviewBoundsWidth\": 1800",
                    "#1975: selected deleted page-header label object JSON should expand deleted preview widths");
    expect_contains(deleted_page_header_object_process.stdout_text, "\"deletedPreviewBoundsHeight\": 2800",
                    "#1975: selected deleted page-header label object JSON should expand deleted preview heights");
    expect_contains(deleted_page_header_object_process.stdout_text, "\"selectedReportSectionAvailable\": false",
                    "#1975: selected deleted page-header label objects should not advertise selected-section availability");
    expect_contains(deleted_page_header_object_process.stdout_text, "\"selectedReportSection\": null",
                    "#1975: selected deleted page-header label objects should serialize null selected sections");
    expect_contains(deleted_page_header_object_process.stdout_text, "\"selectedReportSettingsAvailable\": false",
                    "#1975: selected deleted page-header label objects should not advertise selected-settings availability");
    expect_contains(deleted_page_header_object_process.stdout_text, "\"selectedReportSettings\": null",
                    "#1975: selected deleted page-header label objects should serialize null selected settings");
    expect_contains(deleted_page_header_object_process.stdout_text, "\"liveObjectCount\": 2",
                    "#1975: selected deleted page-header label object JSON should summarize remaining live objects");
    expect_contains(deleted_page_header_object_process.stdout_text, "\"deletedObjectCount\": 2",
                    "#1975: selected deleted page-header label object JSON should summarize deleted objects");
    expect_contains(deleted_page_header_object_process.stdout_text, "\"deletedPlacedObjectCount\": 2",
                    "#1975: selected deleted page-header label object JSON should count deleted placed objects");
    expect_contains(deleted_page_header_object_process.stdout_text, "\"deletedUnplacedObjectCount\": 0",
                    "#1975: selected deleted page-header label object JSON should not fabricate deleted unplaced objects");
    expect_contains_in_order(
        deleted_page_header_object_process.stdout_text,
        {
            "\"selectedReportObject\": {",
            "\"recordIndex\": 4",
            "\"deleted\": true",
            "\"containingSectionId\": \"page_header_1\"",
            "\"containingSectionRecordIndex\": 1",
            "\"sectionRelativeTop\": 100",
            "\"sectionRelativeBottom\": 450",
            "\"sectionObjectIndex\": 0",
            "\"sectionObjectCount\": 1",
            "\"objectTypeCode\": 5",
            "\"objectKind\": \"label\"",
            "\"expression\": \"\\\"Invoice\\\"\""
        },
        "#1975: deleted page-header label object selections should expose selected-object metadata with containing-section membership");
    expect_contains(deleted_page_header_object_process.stdout_text, "\"right\": 2700",
                    "#1975: selected deleted page-header label object JSON should expose object right-edge coordinates");
    expect_contains(deleted_page_header_object_process.stdout_text, "\"bottom\": 450",
                    "#1975: selected deleted page-header label object JSON should expose object bottom-edge coordinates");
    expect_contains(deleted_page_header_object_process.stdout_text, "\"selectedReportObjectSectionAvailable\": true",
                    "#1975: deleted page-header label objects should advertise selected containing-section availability");
    expect_contains_in_order(
        deleted_page_header_object_process.stdout_text,
        {
            "\"selectedReportObjectSection\": {",
            "\"id\": \"page_header_1\"",
            "\"bandKind\": \"page_header\"",
            "\"recordIndex\": 1",
            "\"deleted\": false"
        },
        "#1975: deleted page-header label objects should expose live containing-section JSON");

    const auto deleted_object_process = run_process_capture(
        studio_host_path,
        {"--path", label_path.string(), "--record", "6", "--json"},
        temp_root);

    if (deleted_object_process.exit_code != 0) {
        std::cerr << "studio host selected deleted label object stdout:\n"
                  << deleted_object_process.stdout_text << "\n";
        std::cerr << "studio host selected deleted label object stderr:\n"
                  << deleted_object_process.stderr_text << "\n";
        std::cerr << "fixture root: " << temp_root << "\n";
    }

    expect(deleted_object_process.exit_code == 0,
           "#1500: selected deleted label object JSON should exit successfully");
    expect_contains(deleted_object_process.stdout_text, "\"isLabel\": true",
                    "#1500: selected deleted label object JSON should retain label identity");
    expect_contains(deleted_object_process.stdout_text, "\"selectedReportObjectAvailable\": true",
                    "#1500: deleted label object selections should advertise selected-object availability");
    expect_contains(deleted_object_process.stdout_text, "\"selectedReportSelectionAvailable\": true",
                    "#1500: deleted label object selections should advertise report-selection availability");
    expect_contains(deleted_object_process.stdout_text, "\"selectedReportSelectionKind\": \"object\"",
                    "#1500: deleted label object selections should expose object selection kind");
    expect_contains(deleted_object_process.stdout_text, "\"previewBoundsAvailable\": true",
                    "#1967: selected deleted label object JSON should preserve live preview availability");
    expect_contains(deleted_object_process.stdout_text, "\"previewBoundsLeft\": 0",
                    "#1967: selected deleted label object JSON should preserve live preview left bounds");
    expect_contains(deleted_object_process.stdout_text, "\"previewBoundsTop\": 0",
                    "#1967: selected deleted label object JSON should preserve live preview top bounds");
    expect_contains(deleted_object_process.stdout_text, "\"previewBoundsRight\": 5200",
                    "#1967: selected deleted label object JSON should preserve live preview right bounds");
    expect_contains(deleted_object_process.stdout_text, "\"previewBoundsBottom\": 8100",
                    "#1967: selected deleted label object JSON should preserve live preview bottom bounds");
    expect_contains(deleted_object_process.stdout_text, "\"previewBoundsWidth\": 5200",
                    "#1967: selected deleted label object JSON should preserve live preview widths");
    expect_contains(deleted_object_process.stdout_text, "\"previewBoundsHeight\": 8100",
                    "#1967: selected deleted label object JSON should preserve live preview heights");
    expect_contains(deleted_object_process.stdout_text, "\"deletedPreviewBoundsAvailable\": true",
                    "#1967: selected deleted label object JSON should expose deleted preview availability");
    expect_contains(deleted_object_process.stdout_text, "\"deletedPreviewBoundsLeft\": 1000",
                    "#1967: selected deleted label object JSON should preserve deleted preview left bounds");
    expect_contains(deleted_object_process.stdout_text, "\"deletedPreviewBoundsTop\": 2600",
                    "#1967: selected deleted label object JSON should preserve deleted preview top bounds");
    expect_contains(deleted_object_process.stdout_text, "\"deletedPreviewBoundsRight\": 2200",
                    "#1967: selected deleted label object JSON should preserve deleted preview right bounds");
    expect_contains(deleted_object_process.stdout_text, "\"deletedPreviewBoundsBottom\": 2900",
                    "#1967: selected deleted label object JSON should preserve deleted preview bottom bounds");
    expect_contains(deleted_object_process.stdout_text, "\"deletedPreviewBoundsWidth\": 1200",
                    "#1967: selected deleted label object JSON should preserve deleted preview widths");
    expect_contains(deleted_object_process.stdout_text, "\"deletedPreviewBoundsHeight\": 300",
                    "#1967: selected deleted label object JSON should preserve deleted preview heights");
    expect_contains(deleted_object_process.stdout_text, "\"selectedReportSectionAvailable\": false",
                    "#1507: selected deleted label objects should not advertise selected-section availability");
    expect_contains(deleted_object_process.stdout_text, "\"selectedReportSection\": null",
                    "#1507: selected deleted label objects should serialize null selected sections");
    expect_contains(deleted_object_process.stdout_text, "\"selectedReportSettingsAvailable\": false",
                    "#1507: selected deleted label objects should not advertise selected-settings availability");
    expect_contains(deleted_object_process.stdout_text, "\"selectedReportSettings\": null",
                    "#1507: selected deleted label objects should serialize null selected settings");
    expect_contains(deleted_object_process.stdout_text, "\"deletedObjectCount\": 1",
                    "#1500: deleted selected label object JSON should expose deleted object counts");
    expect_contains_in_order(
        deleted_object_process.stdout_text,
        {
            "\"selectedReportObject\": {",
            "\"recordIndex\": 6",
            "\"deleted\": true",
            "\"containingSectionId\": \"detail_2\"",
            "\"containingSectionRecordIndex\": 2",
            "\"sectionRelativeTop\": 600",
            "\"sectionRelativeBottom\": 900",
            "\"sectionObjectIndex\": 0",
            "\"sectionObjectCount\": 1",
            "\"expression\": \"\\\"Deleted label\\\"\""
        },
        "#1500: deleted label object selections should expose selected deleted-object metadata");
    expect_contains(deleted_object_process.stdout_text, "\"selectedReportObjectSectionAvailable\": true",
                    "#1500: deleted label objects should advertise selected containing-section availability");
    expect_contains_in_order(
        deleted_object_process.stdout_text,
        {
            "\"selectedReportObjectSection\": {",
            "\"id\": \"detail_2\"",
            "\"bandKind\": \"detail\"",
            "\"recordIndex\": 2",
            "\"deleted\": false"
        },
        "#1500: deleted label objects should expose live containing-section JSON");

    const auto unplaced_object_process = run_process_capture(
        studio_host_path,
        {"--path", label_path.string(), "--record", "5", "--json"},
        temp_root);

    if (unplaced_object_process.exit_code != 0) {
        std::cerr << "studio host selected unplaced label object stdout:\n"
                  << unplaced_object_process.stdout_text << "\n";
        std::cerr << "studio host selected unplaced label object stderr:\n"
                  << unplaced_object_process.stderr_text << "\n";
        std::cerr << "fixture root: " << temp_root << "\n";
    }

    expect(unplaced_object_process.exit_code == 0,
           "#1499: selected unplaced label object JSON should exit successfully");
    expect_contains(unplaced_object_process.stdout_text, "\"isLabel\": true",
                    "#1499: selected unplaced label object JSON should retain label identity");
    expect_contains(unplaced_object_process.stdout_text, "\"selectedReportObjectAvailable\": true",
                    "#1499: unplaced label object selections should advertise selected-object availability");
    expect_contains(unplaced_object_process.stdout_text, "\"selectedReportSelectionAvailable\": true",
                    "#1499: unplaced label object selections should advertise report-selection availability");
    expect_contains(unplaced_object_process.stdout_text, "\"selectedReportSelectionKind\": \"object\"",
                    "#1499: unplaced label object selections should expose object selection kind");
    expect_contains(unplaced_object_process.stdout_text, "\"previewBoundsAvailable\": true",
                    "#1969: selected unplaced label object JSON should expose live preview availability");
    expect_contains(unplaced_object_process.stdout_text, "\"previewBoundsLeft\": 0",
                    "#1969: selected unplaced label object JSON should preserve live preview left bounds");
    expect_contains(unplaced_object_process.stdout_text, "\"previewBoundsTop\": 0",
                    "#1969: selected unplaced label object JSON should preserve live preview top bounds");
    expect_contains(unplaced_object_process.stdout_text, "\"previewBoundsRight\": 5200",
                    "#1969: selected unplaced label object JSON should preserve live preview right bounds");
    expect_contains(unplaced_object_process.stdout_text, "\"previewBoundsBottom\": 8100",
                    "#1969: selected unplaced label object JSON should preserve live preview bottom bounds");
    expect_contains(unplaced_object_process.stdout_text, "\"previewBoundsWidth\": 5200",
                    "#1969: selected unplaced label object JSON should preserve live preview widths");
    expect_contains(unplaced_object_process.stdout_text, "\"previewBoundsHeight\": 8100",
                    "#1969: selected unplaced label object JSON should preserve live preview heights");
    expect_contains(unplaced_object_process.stdout_text, "\"deletedPreviewBoundsAvailable\": true",
                    "#1969: selected unplaced label object JSON should expose deleted preview availability");
    expect_contains(unplaced_object_process.stdout_text, "\"deletedPreviewBoundsLeft\": 1000",
                    "#1969: selected unplaced label object JSON should preserve deleted preview left bounds");
    expect_contains(unplaced_object_process.stdout_text, "\"deletedPreviewBoundsTop\": 2600",
                    "#1969: selected unplaced label object JSON should preserve deleted preview top bounds");
    expect_contains(unplaced_object_process.stdout_text, "\"deletedPreviewBoundsRight\": 2200",
                    "#1969: selected unplaced label object JSON should preserve deleted preview right bounds");
    expect_contains(unplaced_object_process.stdout_text, "\"deletedPreviewBoundsBottom\": 2900",
                    "#1969: selected unplaced label object JSON should preserve deleted preview bottom bounds");
    expect_contains(unplaced_object_process.stdout_text, "\"deletedPreviewBoundsWidth\": 1200",
                    "#1969: selected unplaced label object JSON should preserve deleted preview widths");
    expect_contains(unplaced_object_process.stdout_text, "\"deletedPreviewBoundsHeight\": 300",
                    "#1969: selected unplaced label object JSON should preserve deleted preview heights");
    expect_contains(unplaced_object_process.stdout_text, "\"selectedReportSectionAvailable\": false",
                    "#1508: selected unplaced label objects should not advertise selected-section availability");
    expect_contains(unplaced_object_process.stdout_text, "\"selectedReportSection\": null",
                    "#1508: selected unplaced label objects should serialize null selected sections");
    expect_contains(unplaced_object_process.stdout_text, "\"selectedReportSettingsAvailable\": false",
                    "#1508: selected unplaced label objects should not advertise selected-settings availability");
    expect_contains(unplaced_object_process.stdout_text, "\"selectedReportSettings\": null",
                    "#1508: selected unplaced label objects should serialize null selected settings");
    expect_contains(unplaced_object_process.stdout_text, "\"unplacedObjectCount\": 1",
                    "#1499: unplaced selected label object JSON should expose unplaced object counts");
    expect_contains_in_order(
        unplaced_object_process.stdout_text,
        {
            "\"selectedReportObject\": {",
            "\"recordIndex\": 5",
            "\"deleted\": false",
            "\"containingSectionId\": \"\"",
            "\"containingSectionRecordIndex\": null",
            "\"sectionObjectIndex\": null",
            "\"objectKind\": \"line\""
        },
        "#1499: unplaced label object selections should expose selected-object metadata without section membership");
    expect_contains(unplaced_object_process.stdout_text, "\"selectedReportObjectSectionAvailable\": false",
                    "#1499: unplaced label objects should not advertise selected containing-section availability");
    expect_contains(unplaced_object_process.stdout_text, "\"selectedReportObjectSection\": null",
                    "#1499: unplaced label objects should serialize null selected containing-section JSON");

    const fs::path deleted_unplaced_object_path = temp_root / "deleted_unplaced_label_object.lbx";
    write_synthetic_report_table_for_layout_json(deleted_unplaced_object_path);
    const auto delete_unplaced_object_result =
        copperfin::vfp::set_record_deleted_flag(deleted_unplaced_object_path.string(), 5U, true);
    expect(delete_unplaced_object_result.ok,
           "#1971: selected deleted unplaced label object fixture should mark the unplaced object deleted");

    const auto deleted_unplaced_object_process = run_process_capture(
        studio_host_path,
        {"--path", deleted_unplaced_object_path.string(), "--record", "5", "--json"},
        temp_root);

    if (deleted_unplaced_object_process.exit_code != 0) {
        std::cerr << "studio host selected deleted unplaced label object stdout:\n"
                  << deleted_unplaced_object_process.stdout_text << "\n";
        std::cerr << "studio host selected deleted unplaced label object stderr:\n"
                  << deleted_unplaced_object_process.stderr_text << "\n";
        std::cerr << "fixture root: " << temp_root << "\n";
    }

    expect(deleted_unplaced_object_process.exit_code == 0,
           "#1971: selected deleted unplaced label object JSON should exit successfully");
    expect_contains(deleted_unplaced_object_process.stdout_text, "\"isLabel\": true",
                    "#1971: selected deleted unplaced label object JSON should retain label identity");
    expect_contains(deleted_unplaced_object_process.stdout_text, "\"selectedReportObjectAvailable\": true",
                    "#1971: deleted unplaced label object selections should advertise selected-object availability");
    expect_contains(deleted_unplaced_object_process.stdout_text, "\"selectedReportSelectionAvailable\": true",
                    "#1971: deleted unplaced label object selections should advertise report-selection availability");
    expect_contains(deleted_unplaced_object_process.stdout_text, "\"selectedReportSelectionKind\": \"object\"",
                    "#1971: deleted unplaced label object selections should expose object selection kind");
    expect_contains(deleted_unplaced_object_process.stdout_text, "\"previewBoundsAvailable\": true",
                    "#1971: selected deleted unplaced label object JSON should preserve live preview availability");
    expect_contains(deleted_unplaced_object_process.stdout_text, "\"previewBoundsLeft\": 0",
                    "#1971: selected deleted unplaced label object JSON should preserve live preview left bounds");
    expect_contains(deleted_unplaced_object_process.stdout_text, "\"previewBoundsTop\": 0",
                    "#1971: selected deleted unplaced label object JSON should preserve live preview top bounds");
    expect_contains(deleted_unplaced_object_process.stdout_text, "\"previewBoundsRight\": 5200",
                    "#1971: selected deleted unplaced label object JSON should preserve live preview right bounds");
    expect_contains(deleted_unplaced_object_process.stdout_text, "\"previewBoundsBottom\": 7000",
                    "#1971: selected deleted unplaced label object JSON should preserve remaining live preview bottom bounds");
    expect_contains(deleted_unplaced_object_process.stdout_text, "\"previewBoundsWidth\": 5200",
                    "#1971: selected deleted unplaced label object JSON should preserve live preview widths");
    expect_contains(deleted_unplaced_object_process.stdout_text, "\"previewBoundsHeight\": 7000",
                    "#1971: selected deleted unplaced label object JSON should preserve remaining live preview heights");
    expect_contains(deleted_unplaced_object_process.stdout_text, "\"deletedPreviewBoundsAvailable\": true",
                    "#1971: selected deleted unplaced label object JSON should expose deleted preview availability");
    expect_contains(deleted_unplaced_object_process.stdout_text, "\"deletedPreviewBoundsLeft\": 50",
                    "#1971: selected deleted unplaced label object JSON should expand deleted preview left bounds");
    expect_contains(deleted_unplaced_object_process.stdout_text, "\"deletedPreviewBoundsTop\": 2600",
                    "#1971: selected deleted unplaced label object JSON should preserve deleted preview top bounds");
    expect_contains(deleted_unplaced_object_process.stdout_text, "\"deletedPreviewBoundsRight\": 2200",
                    "#1971: selected deleted unplaced label object JSON should preserve deleted preview right bounds");
    expect_contains(deleted_unplaced_object_process.stdout_text, "\"deletedPreviewBoundsBottom\": 8100",
                    "#1971: selected deleted unplaced label object JSON should expand deleted preview bottom bounds");
    expect_contains(deleted_unplaced_object_process.stdout_text, "\"deletedPreviewBoundsWidth\": 2150",
                    "#1971: selected deleted unplaced label object JSON should expand deleted preview widths");
    expect_contains(deleted_unplaced_object_process.stdout_text, "\"deletedPreviewBoundsHeight\": 5500",
                    "#1971: selected deleted unplaced label object JSON should expand deleted preview heights");
    expect_contains(deleted_unplaced_object_process.stdout_text, "\"selectedReportSectionAvailable\": false",
                    "#1971: selected deleted unplaced label objects should not advertise selected-section availability");
    expect_contains(deleted_unplaced_object_process.stdout_text, "\"selectedReportSection\": null",
                    "#1971: selected deleted unplaced label objects should serialize null selected sections");
    expect_contains(deleted_unplaced_object_process.stdout_text, "\"selectedReportSettingsAvailable\": false",
                    "#1971: selected deleted unplaced label objects should not advertise selected-settings availability");
    expect_contains(deleted_unplaced_object_process.stdout_text, "\"selectedReportSettings\": null",
                    "#1971: selected deleted unplaced label objects should serialize null selected settings");
    expect_contains(deleted_unplaced_object_process.stdout_text, "\"liveObjectCount\": 2",
                    "#1971: selected deleted unplaced label object JSON should summarize remaining live objects");
    expect_contains(deleted_unplaced_object_process.stdout_text, "\"deletedObjectCount\": 2",
                    "#1971: selected deleted unplaced label object JSON should summarize deleted objects");
    expect_contains(deleted_unplaced_object_process.stdout_text, "\"deletedPlacedObjectCount\": 1",
                    "#1971: selected deleted unplaced label object JSON should retain deleted placed object counts");
    expect_contains(deleted_unplaced_object_process.stdout_text, "\"deletedUnplacedObjectCount\": 1",
                    "#1971: selected deleted unplaced label object JSON should count deleted unplaced objects");
    expect_contains(deleted_unplaced_object_process.stdout_text, "\"left\": 50",
                    "#1971: deleted unplaced label object selections should expose selected-object left bounds");
    expect_contains(deleted_unplaced_object_process.stdout_text, "\"top\": 8000",
                    "#1971: deleted unplaced label object selections should expose selected-object top bounds");
    expect_contains(deleted_unplaced_object_process.stdout_text, "\"right\": 150",
                    "#1971: deleted unplaced label object selections should expose selected-object right bounds");
    expect_contains(deleted_unplaced_object_process.stdout_text, "\"bottom\": 8100",
                    "#1971: deleted unplaced label object selections should expose selected-object bottom bounds");
    expect_contains_in_order(
        deleted_unplaced_object_process.stdout_text,
        {
            "\"selectedReportObject\": {",
            "\"recordIndex\": 5",
            "\"deleted\": true",
            "\"containingSectionId\": \"\"",
            "\"containingSectionRecordIndex\": null",
            "\"sectionObjectIndex\": null",
            "\"sectionObjectCount\": 0",
            "\"objectKind\": \"line\""
        },
        "#1971: deleted unplaced label object selections should expose selected-object metadata without section membership");
    expect_contains(deleted_unplaced_object_process.stdout_text, "\"selectedReportObjectSectionAvailable\": false",
                    "#1971: deleted unplaced label objects should not advertise selected containing-section availability");
    expect_contains(deleted_unplaced_object_process.stdout_text, "\"selectedReportObjectSection\": null",
                    "#1971: deleted unplaced label objects should serialize null selected containing-section JSON");

    const auto section_process = run_process_capture(
        studio_host_path,
        {"--path", label_path.string(), "--record", "2", "--json"},
        temp_root);

    if (section_process.exit_code != 0) {
        std::cerr << "studio host label layout section stdout:\n" << section_process.stdout_text << "\n";
        std::cerr << "studio host label layout section stderr:\n" << section_process.stderr_text << "\n";
        std::cerr << "fixture root: " << temp_root << "\n";
    }

    expect(section_process.exit_code == 0,
           "#1481: label layout JSON should exit successfully for selected label sections");
    expect_contains(section_process.stdout_text, "\"isLabel\": true",
                    "#1502: selected label sections should retain label identity");
    expect_contains(section_process.stdout_text, "\"selectedReportSelectionKind\": \"section\"",
                    "#1481: selected label sections should reuse report section selection kind");
    expect_contains(section_process.stdout_text, "\"selectedReportSectionAvailable\": true",
                    "#1481: selected label sections should expose selected section JSON");
    expect_contains(section_process.stdout_text, "\"previewBoundsAvailable\": true",
                    "#1962: selected label section JSON should expose live preview availability");
    expect_contains(section_process.stdout_text, "\"previewBoundsLeft\": 0",
                    "#1962: selected label section JSON should preserve live preview left bounds");
    expect_contains(section_process.stdout_text, "\"previewBoundsTop\": 0",
                    "#1962: selected label section JSON should preserve live preview top bounds");
    expect_contains(section_process.stdout_text, "\"previewBoundsRight\": 5200",
                    "#1962: selected label section JSON should preserve live preview right bounds");
    expect_contains(section_process.stdout_text, "\"previewBoundsBottom\": 8100",
                    "#1962: selected label section JSON should preserve live preview bottom bounds");
    expect_contains(section_process.stdout_text, "\"previewBoundsWidth\": 5200",
                    "#1962: selected label section JSON should preserve live preview widths");
    expect_contains(section_process.stdout_text, "\"previewBoundsHeight\": 8100",
                    "#1962: selected label section JSON should preserve live preview heights");
    expect_contains(section_process.stdout_text, "\"deletedPreviewBoundsAvailable\": true",
                    "#1962: selected label section JSON should expose deleted preview availability");
    expect_contains(section_process.stdout_text, "\"deletedPreviewBoundsLeft\": 1000",
                    "#1962: selected label section JSON should preserve deleted preview left bounds");
    expect_contains(section_process.stdout_text, "\"deletedPreviewBoundsTop\": 2600",
                    "#1962: selected label section JSON should preserve deleted preview top bounds");
    expect_contains(section_process.stdout_text, "\"deletedPreviewBoundsRight\": 2200",
                    "#1962: selected label section JSON should preserve deleted preview right bounds");
    expect_contains(section_process.stdout_text, "\"deletedPreviewBoundsBottom\": 2900",
                    "#1962: selected label section JSON should preserve deleted preview bottom bounds");
    expect_contains(section_process.stdout_text, "\"deletedPreviewBoundsWidth\": 1200",
                    "#1962: selected label section JSON should preserve deleted preview widths");
    expect_contains(section_process.stdout_text, "\"deletedPreviewBoundsHeight\": 300",
                    "#1962: selected label section JSON should preserve deleted preview heights");
    expect_contains(section_process.stdout_text, "\"selectedReportObjectAvailable\": false",
                    "#1502: selected label sections should not advertise selected-object availability");
    expect_contains(section_process.stdout_text, "\"selectedReportObject\": null",
                    "#1502: selected label sections should serialize null selected objects");
    expect_contains(section_process.stdout_text, "\"selectedReportObjectSectionAvailable\": false",
                    "#1502: selected label sections should not advertise containing-object-section availability");
    expect_contains(section_process.stdout_text, "\"selectedReportObjectSection\": null",
                    "#1502: selected label sections should serialize null containing-object sections");
    expect_contains(section_process.stdout_text, "\"selectedReportSettingsAvailable\": false",
                    "#1962: selected label sections should not advertise selected-settings availability");
    expect_contains(section_process.stdout_text, "\"selectedReportSettings\": null",
                    "#1962: selected label sections should serialize null selected settings");
    expect_contains(section_process.stdout_text, "\"bandKind\": \"detail\"",
                    "#1481: selected label sections should expose band metadata");
    expect_contains(section_process.stdout_text, "\"objectCount\": 1",
                    "#1481: selected label sections should expose section object counts");

    const fs::path deleted_section_path = temp_root / "deleted_section.lbx";
    write_synthetic_report_table_for_deleted_section_json(deleted_section_path);
    const auto deleted_section_process = run_process_capture(
        studio_host_path,
        {"--path", deleted_section_path.string(), "--record", "1", "--json"},
        temp_root);

    if (deleted_section_process.exit_code != 0) {
        std::cerr << "studio host selected deleted label section stdout:\n"
                  << deleted_section_process.stdout_text << "\n";
        std::cerr << "studio host selected deleted label section stderr:\n"
                  << deleted_section_process.stderr_text << "\n";
        std::cerr << "fixture root: " << temp_root << "\n";
    }

    expect(deleted_section_process.exit_code == 0,
           "#1498: selected deleted label section JSON should exit successfully");
    expect_contains(deleted_section_process.stdout_text, "\"isLabel\": true",
                    "#1498: selected deleted label section JSON should retain label identity");
    expect_contains(deleted_section_process.stdout_text, "\"selectedReportSectionAvailable\": true",
                    "#1498: deleted label section selections should advertise selected-section availability");
    expect_contains(deleted_section_process.stdout_text, "\"selectedReportSelectionAvailable\": true",
                    "#1498: deleted label section selections should advertise report-selection availability");
    expect_contains(deleted_section_process.stdout_text, "\"selectedReportSelectionKind\": \"section\"",
                    "#1498: deleted label section selections should expose section selection kind");
    expect_contains(deleted_section_process.stdout_text, "\"selectedReportObjectAvailable\": false",
                    "#1503: selected deleted label sections should not advertise selected-object availability");
    expect_contains(deleted_section_process.stdout_text, "\"selectedReportObject\": null",
                    "#1503: selected deleted label sections should serialize null selected objects");
    expect_contains(deleted_section_process.stdout_text, "\"selectedReportObjectSectionAvailable\": false",
                    "#1503: selected deleted label sections should not advertise containing-object-section availability");
    expect_contains(deleted_section_process.stdout_text, "\"selectedReportObjectSection\": null",
                    "#1503: selected deleted label sections should serialize null containing-object sections");
    expect_contains(deleted_section_process.stdout_text, "\"selectedReportSettingsAvailable\": false",
                    "#1963: selected deleted label sections should not advertise selected-settings availability");
    expect_contains(deleted_section_process.stdout_text, "\"selectedReportSettings\": null",
                    "#1963: selected deleted label sections should serialize null selected settings");
    expect_contains(deleted_section_process.stdout_text, "\"sectionCount\": 0",
                    "#1498: deleted selected label section JSON should not expose live sections");
    expect_contains(deleted_section_process.stdout_text, "\"deletedSectionCount\": 1",
                    "#1498: deleted selected label section JSON should expose deleted section counts");
    expect_contains(deleted_section_process.stdout_text, "\"previewBoundsAvailable\": true",
                    "#1963: deleted selected label section JSON should preserve live preview availability");
    expect_contains(deleted_section_process.stdout_text, "\"previewBoundsLeft\": 100",
                    "#1963: deleted selected label section JSON should preserve remaining live preview left bounds");
    expect_contains(deleted_section_process.stdout_text, "\"previewBoundsTop\": 2600",
                    "#1963: deleted selected label section JSON should preserve remaining live preview top bounds");
    expect_contains(deleted_section_process.stdout_text, "\"previewBoundsRight\": 150",
                    "#1963: deleted selected label section JSON should preserve remaining live preview right bounds");
    expect_contains(deleted_section_process.stdout_text, "\"previewBoundsBottom\": 2800",
                    "#1963: deleted selected label section JSON should preserve remaining live preview bottom bounds");
    expect_contains(deleted_section_process.stdout_text, "\"previewBoundsWidth\": 50",
                    "#1963: deleted selected label section JSON should preserve remaining live preview widths");
    expect_contains(deleted_section_process.stdout_text, "\"previewBoundsHeight\": 200",
                    "#1963: deleted selected label section JSON should preserve remaining live preview heights");
    expect_contains(deleted_section_process.stdout_text, "\"deletedPreviewBoundsAvailable\": true",
                    "#1963: deleted selected label section JSON should expose deleted preview availability");
    expect_contains(deleted_section_process.stdout_text, "\"deletedPreviewBoundsLeft\": 0",
                    "#1963: deleted selected label section JSON should preserve deleted preview left bounds");
    expect_contains(deleted_section_process.stdout_text, "\"deletedPreviewBoundsTop\": 2000",
                    "#1963: deleted selected label section JSON should preserve deleted preview top bounds");
    expect_contains(deleted_section_process.stdout_text, "\"deletedPreviewBoundsRight\": 0",
                    "#1963: deleted selected label section JSON should preserve deleted preview right bounds");
    expect_contains(deleted_section_process.stdout_text, "\"deletedPreviewBoundsBottom\": 7000",
                    "#1963: deleted selected label section JSON should preserve deleted preview bottom bounds");
    expect_contains(deleted_section_process.stdout_text, "\"deletedPreviewBoundsWidth\": 0",
                    "#1963: deleted selected label section JSON should preserve deleted preview widths");
    expect_contains(deleted_section_process.stdout_text, "\"deletedPreviewBoundsHeight\": 5000",
                    "#1963: deleted selected label section JSON should preserve deleted preview heights");
    expect_contains_in_order(
        deleted_section_process.stdout_text,
        {
            "\"selectedReportSection\": {",
            "\"bandKind\": \"detail\"",
            "\"recordIndex\": 1",
            "\"deleted\": true",
            "\"sectionIndex\": null",
            "\"sectionCount\": 0"
        },
        "#1498: deleted label section selections should expose selected deleted-section metadata");
    expect_contains(deleted_section_process.stdout_text, "\"unplacedObjectCount\": 3",
                    "#1498: deleted selected label section JSON should preserve former section objects as unplaced");
    expect_contains(deleted_section_process.stdout_text, "\"containingSectionId\": \"\"",
                    "#1498: deleted selected label section objects should not fabricate containing-section ids");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_nudges_label_layout_objects_by_stable_selectors(const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_label_layout_nudge_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path label_path = temp_root / "mailing.lbx";
    write_synthetic_report_table_for_layout_json(label_path);

    const auto nudge_process = run_process_capture(
        studio_host_path,
        {
            "--path", label_path.string(),
            "--record", "3",
            "--nudge-object",
            "--nudge-mode", "both",
            "--delta-hpos", "50",
            "--delta-vpos", "-200",
            "--nudge-target-unique-id", "field-guid",
            "--json"
        },
        temp_root);

    if (nudge_process.exit_code != 0) {
        std::cerr << "studio host label object nudge stdout:\n" << nudge_process.stdout_text << "\n";
        std::cerr << "studio host label object nudge stderr:\n" << nudge_process.stderr_text << "\n";
        std::cerr << "fixture root: " << temp_root << "\n";
    }

    expect(nudge_process.exit_code == 0,
           "#1482: label layout object nudge should exit successfully");
    expect(visual_object_property(label_path, "field-guid", "HPOS") == "1250" &&
               visual_object_property(label_path, "field-guid", "VPOS") == "2400",
           "#1482: label layout object nudge should mutate LBX HPOS and VPOS fields");
    expect_contains(nudge_process.stdout_text, "\"isLabel\": true",
                    "#1482: nudged label layout JSON should retain label identity");
    expect_contains(nudge_process.stdout_text, "\"selectedReportObjectAvailable\": true",
                    "#1482: nudged label object JSON should retain selected-object availability");
    expect_contains(nudge_process.stdout_text, "\"left\": 1250",
                    "#1482: nudged label object JSON should expose updated left coordinates");
    expect_contains(nudge_process.stdout_text, "\"top\": 2400",
                    "#1482: nudged label object JSON should expose updated top coordinates");
    expect_contains(nudge_process.stdout_text, "\"right\": 5250",
                    "#1482: nudged label object JSON should recompute right-edge coordinates");
    expect_contains(nudge_process.stdout_text, "\"bottom\": 2850",
                    "#1482: nudged label object JSON should recompute bottom-edge coordinates");
    expect_contains(nudge_process.stdout_text, "\"sectionRelativeTop\": 400",
                    "#1482: nudged label object JSON should recompute section-relative top coordinates");
    expect_contains(nudge_process.stdout_text, "\"sectionRelativeBottom\": 850",
                    "#1482: nudged label object JSON should recompute section-relative bottom coordinates");
    expect_contains(nudge_process.stdout_text, "\"containingSectionId\": \"detail_2\"",
                    "#1482: nudged label object JSON should preserve containing section metadata");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_aligns_label_layout_objects_by_stable_selectors(const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_label_layout_align_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path label_path = temp_root / "mailing.lbx";
    write_synthetic_report_table_for_layout_json(label_path);

    const auto align_process = run_process_capture(
        studio_host_path,
        {
            "--path", label_path.string(),
            "--record", "3",
            "--align-object",
            "--alignment-mode", "left",
            "--anchor-unique-id", "label-guid",
            "--align-target-unique-id", "field-guid",
            "--json"
        },
        temp_root);

    if (align_process.exit_code != 0) {
        std::cerr << "studio host label object align stdout:\n" << align_process.stdout_text << "\n";
        std::cerr << "studio host label object align stderr:\n" << align_process.stderr_text << "\n";
        std::cerr << "fixture root: " << temp_root << "\n";
    }

    expect(align_process.exit_code == 0,
           "#1483: label layout object alignment should exit successfully");
    expect(visual_object_property(label_path, "field-guid", "HPOS") == "900" &&
               visual_object_property(label_path, "field-guid", "VPOS") == "2600",
           "#1483: label layout object left alignment should mutate LBX HPOS and preserve VPOS");
    expect(visual_object_property(label_path, "label-guid", "HPOS") == "900",
           "#1483: label layout object alignment should preserve anchor geometry");
    expect_contains(align_process.stdout_text, "\"isLabel\": true",
                    "#1483: aligned label layout JSON should retain label identity");
    expect_contains(align_process.stdout_text, "\"selectedReportObjectAvailable\": true",
                    "#1483: aligned label object JSON should retain selected-object availability");
    expect_contains(align_process.stdout_text, "\"left\": 900",
                    "#1483: aligned label object JSON should expose updated left coordinates");
    expect_contains(align_process.stdout_text, "\"top\": 2600",
                    "#1483: aligned label object JSON should preserve top coordinates");
    expect_contains(align_process.stdout_text, "\"right\": 4900",
                    "#1483: aligned label object JSON should recompute right-edge coordinates");
    expect_contains(align_process.stdout_text, "\"bottom\": 3050",
                    "#1483: aligned label object JSON should preserve bottom-edge coordinates");
    expect_contains(align_process.stdout_text, "\"sectionRelativeTop\": 600",
                    "#1483: aligned label object JSON should preserve section-relative top coordinates");
    expect_contains(align_process.stdout_text, "\"containingSectionId\": \"detail_2\"",
                    "#1483: aligned label object JSON should preserve containing section metadata");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_resizes_label_layout_objects_by_stable_selectors(const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_label_layout_resize_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path label_path = temp_root / "mailing.lbx";
    write_synthetic_report_table_for_layout_json(label_path);

    const auto resize_process = run_process_capture(
        studio_host_path,
        {
            "--path", label_path.string(),
            "--record", "3",
            "--resize-object",
            "--resize-mode", "size",
            "--anchor-unique-id", "label-guid",
            "--resize-target-unique-id", "field-guid",
            "--json"
        },
        temp_root);

    if (resize_process.exit_code != 0) {
        std::cerr << "studio host label object resize stdout:\n" << resize_process.stdout_text << "\n";
        std::cerr << "studio host label object resize stderr:\n" << resize_process.stderr_text << "\n";
        std::cerr << "fixture root: " << temp_root << "\n";
    }

    expect(resize_process.exit_code == 0,
           "#1484: label layout object resize should exit successfully");
    expect(visual_object_property(label_path, "field-guid", "WIDTH") == "1800" &&
               visual_object_property(label_path, "field-guid", "HEIGHT") == "350",
           "#1484: label layout object size resize should mutate LBX WIDTH and HEIGHT fields");
    expect(visual_object_property(label_path, "field-guid", "HPOS") == "1200" &&
               visual_object_property(label_path, "field-guid", "VPOS") == "2600",
           "#1484: label layout object size resize should preserve LBX HPOS and VPOS fields");
    expect_contains(resize_process.stdout_text, "\"isLabel\": true",
                    "#1484: resized label layout JSON should retain label identity");
    expect_contains(resize_process.stdout_text, "\"selectedReportObjectAvailable\": true",
                    "#1484: resized label object JSON should retain selected-object availability");
    expect_contains(resize_process.stdout_text, "\"left\": 1200",
                    "#1484: resized label object JSON should preserve left coordinates");
    expect_contains(resize_process.stdout_text, "\"top\": 2600",
                    "#1484: resized label object JSON should preserve top coordinates");
    expect_contains(resize_process.stdout_text, "\"width\": 1800",
                    "#1484: resized label object JSON should expose updated width");
    expect_contains(resize_process.stdout_text, "\"height\": 350",
                    "#1484: resized label object JSON should expose updated height");
    expect_contains(resize_process.stdout_text, "\"right\": 3000",
                    "#1484: resized label object JSON should recompute right-edge coordinates");
    expect_contains(resize_process.stdout_text, "\"bottom\": 2950",
                    "#1484: resized label object JSON should recompute bottom-edge coordinates");
    expect_contains(resize_process.stdout_text, "\"sectionRelativeTop\": 600",
                    "#1484: resized label object JSON should preserve section-relative top coordinates");
    expect_contains(resize_process.stdout_text, "\"sectionRelativeBottom\": 950",
                    "#1484: resized label object JSON should recompute section-relative bottom coordinates");
    expect_contains(resize_process.stdout_text, "\"containingSectionId\": \"detail_2\"",
                    "#1484: resized label object JSON should preserve containing section metadata");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_snaps_label_layout_objects_by_stable_selectors(const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_label_layout_snap_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path label_path = temp_root / "mailing.lbx";
    write_synthetic_report_table_for_layout_json(label_path);

    const auto snap_process = run_process_capture(
        studio_host_path,
        {
            "--path", label_path.string(),
            "--record", "3",
            "--snap-object",
            "--snap-mode", "both",
            "--grid-width", "700",
            "--grid-height", "750",
            "--snap-target-unique-id", "field-guid",
            "--json"
        },
        temp_root);

    if (snap_process.exit_code != 0) {
        std::cerr << "studio host label object snap stdout:\n" << snap_process.stdout_text << "\n";
        std::cerr << "studio host label object snap stderr:\n" << snap_process.stderr_text << "\n";
        std::cerr << "fixture root: " << temp_root << "\n";
    }

    expect(snap_process.exit_code == 0,
           "#1485: label layout object snap should exit successfully");
    expect(visual_object_property(label_path, "field-guid", "HPOS") == "1400" &&
               visual_object_property(label_path, "field-guid", "VPOS") == "2250",
           "#1485: label layout object snap should mutate LBX HPOS and VPOS fields");
    expect(visual_object_property(label_path, "field-guid", "WIDTH") == "4000" &&
               visual_object_property(label_path, "field-guid", "HEIGHT") == "450",
           "#1485: label layout object snap should preserve LBX WIDTH and HEIGHT fields");
    expect_contains(snap_process.stdout_text, "\"isLabel\": true",
                    "#1485: snapped label layout JSON should retain label identity");
    expect_contains(snap_process.stdout_text, "\"selectedReportObjectAvailable\": true",
                    "#1485: snapped label object JSON should retain selected-object availability");
    expect_contains(snap_process.stdout_text, "\"left\": 1400",
                    "#1485: snapped label object JSON should expose updated left coordinates");
    expect_contains(snap_process.stdout_text, "\"top\": 2250",
                    "#1485: snapped label object JSON should expose updated top coordinates");
    expect_contains(snap_process.stdout_text, "\"width\": 4000",
                    "#1485: snapped label object JSON should preserve width");
    expect_contains(snap_process.stdout_text, "\"height\": 450",
                    "#1485: snapped label object JSON should preserve height");
    expect_contains(snap_process.stdout_text, "\"right\": 5400",
                    "#1485: snapped label object JSON should recompute right-edge coordinates");
    expect_contains(snap_process.stdout_text, "\"bottom\": 2700",
                    "#1485: snapped label object JSON should recompute bottom-edge coordinates");
    expect_contains(snap_process.stdout_text, "\"sectionRelativeTop\": 250",
                    "#1485: snapped label object JSON should recompute section-relative top coordinates");
    expect_contains(snap_process.stdout_text, "\"sectionRelativeBottom\": 700",
                    "#1485: snapped label object JSON should recompute section-relative bottom coordinates");
    expect_contains(snap_process.stdout_text, "\"containingSectionId\": \"detail_2\"",
                    "#1485: snapped label object JSON should preserve containing section metadata");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_deletes_label_layout_objects_by_stable_selectors(const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_label_layout_delete_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path label_path = temp_root / "mailing.lbx";
    write_synthetic_report_table_for_layout_json(label_path);

    const auto delete_process = run_process_capture(
        studio_host_path,
        {
            "--path", label_path.string(),
            "--record", "3",
            "--delete-object",
            "--unique-id", "field-guid",
            "--json"
        },
        temp_root);

    if (delete_process.exit_code != 0) {
        std::cerr << "studio host label object delete stdout:\n" << delete_process.stdout_text << "\n";
        std::cerr << "studio host label object delete stderr:\n" << delete_process.stderr_text << "\n";
        std::cerr << "fixture root: " << temp_root << "\n";
    }

    expect(delete_process.exit_code == 0,
           "#1486: label layout object delete should exit successfully");
    expect(visual_object_deleted(label_path, "field-guid"),
           "#1486: label layout object delete should mark the LBX object record deleted");
    expect_contains(delete_process.stdout_text, "\"isLabel\": true",
                    "#1486: deleted label layout JSON should retain label identity");
    expect_contains(delete_process.stdout_text, "\"deletedObjectCount\": 2",
                    "#1486: deleted label object JSON should move the object into deleted label objects");
    expect_contains(delete_process.stdout_text, "\"selectedReportObjectAvailable\": true",
                    "#1486: deleted selected label object JSON should remain available");
    expect_contains(delete_process.stdout_text, "\"selectedReportObjectSectionAvailable\": false",
                    "#1486: deleted label objects should not advertise containing-section availability");
    expect_contains(delete_process.stdout_text, "\"selectedReportObjectSection\": null",
                    "#1486: deleted label objects should serialize null containing-section JSON");
    expect_contains(delete_process.stdout_text, "\"selectedReportSelectionKind\": \"object\"",
                    "#1486: deleted label object selections should still classify as report objects");
    expect_contains(delete_process.stdout_text, "\"recordIndex\": 3",
                    "#1486: deleted selected label object JSON should preserve selected record indexes");
    expect_contains(delete_process.stdout_text, "\"deleted\": true",
                    "#1486: deleted selected label object JSON should expose deleted state");
    expect_contains(delete_process.stdout_text, "\"containingSectionRecordIndex\": null",
                    "#1486: deleted label object JSON should not fabricate containing section record indexes");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_restores_label_layout_objects_by_stable_selectors(const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_label_layout_restore_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path label_path = temp_root / "mailing.lbx";
    write_synthetic_report_table_for_layout_json(label_path);
    const auto seed_delete_result = copperfin::vfp::set_visual_object_deleted_state({
        .path = label_path.string(),
        .record_index = 0U,
        .object_name = {},
        .unique_id = "field-guid",
        .deleted = true
    });
    expect(seed_delete_result.ok && visual_object_deleted(label_path, "field-guid"),
           "#1487: label layout restore fixture should start with a deleted label object");

    const auto restore_process = run_process_capture(
        studio_host_path,
        {
            "--path", label_path.string(),
            "--record", "3",
            "--restore-object",
            "--unique-id", "field-guid",
            "--json"
        },
        temp_root);

    if (restore_process.exit_code != 0) {
        std::cerr << "studio host label object restore stdout:\n" << restore_process.stdout_text << "\n";
        std::cerr << "studio host label object restore stderr:\n" << restore_process.stderr_text << "\n";
        std::cerr << "fixture root: " << temp_root << "\n";
    }

    expect(restore_process.exit_code == 0,
           "#1487: label layout object restore should exit successfully");
    expect(!visual_object_deleted(label_path, "field-guid"),
           "#1487: label layout object restore should clear the LBX object deleted flag");
    expect_contains(restore_process.stdout_text, "\"isLabel\": true",
                    "#1487: restored label layout JSON should retain label identity");
    expect_contains(restore_process.stdout_text, "\"deletedObjectCount\": 1",
                    "#1487: restored label object JSON should move the object out of deleted label objects");
    expect_contains(restore_process.stdout_text, "\"selectedReportObjectAvailable\": true",
                    "#1487: restored selected label object JSON should remain available");
    expect_contains(restore_process.stdout_text, "\"selectedReportObjectSectionAvailable\": true",
                    "#1487: restored label objects should advertise containing-section availability");
    expect_contains(restore_process.stdout_text, "\"selectedReportObjectSection\": {",
                    "#1487: restored label objects should serialize containing-section JSON");
    expect_contains(restore_process.stdout_text, "\"selectedReportSelectionKind\": \"object\"",
                    "#1487: restored label object selections should classify as report objects");
    expect_contains(restore_process.stdout_text, "\"recordIndex\": 3",
                    "#1487: restored selected label object JSON should preserve selected record indexes");
    expect_contains(restore_process.stdout_text, "\"deleted\": false",
                    "#1487: restored selected label object JSON should expose live state");
    expect_contains(restore_process.stdout_text, "\"containingSectionId\": \"detail_2\"",
                    "#1487: restored label object JSON should expose containing section ids again");
    expect_contains(restore_process.stdout_text, "\"containingSectionRecordIndex\": 2",
                    "#1487: restored label object JSON should expose containing section record indexes again");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_distributes_label_layout_objects_by_stable_selectors(const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_label_layout_distribute_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path label_path = temp_root / "mailing.lbx";
    write_synthetic_report_table_for_layout_distribution_json(label_path);

    const auto distribute_process = run_process_capture(
        studio_host_path,
        {
            "--path", label_path.string(),
            "--record", "3",
            "--distribute-object",
            "--distribution-mode", "horizontal",
            "--distribute-target-unique-id", "left-field-guid",
            "--distribute-target-unique-id", "middle-field-guid",
            "--distribute-target-unique-id", "right-field-guid",
            "--json"
        },
        temp_root);

    if (distribute_process.exit_code != 0) {
        std::cerr << "studio host label object distribute stdout:\n" << distribute_process.stdout_text << "\n";
        std::cerr << "studio host label object distribute stderr:\n" << distribute_process.stderr_text << "\n";
        std::cerr << "fixture root: " << temp_root << "\n";
    }

    expect(distribute_process.exit_code == 0,
           "#1488: label layout object distribution should exit successfully");
    expect(visual_object_property(label_path, "left-field-guid", "HPOS") == "100" &&
               visual_object_property(label_path, "middle-field-guid", "HPOS") == "400" &&
               visual_object_property(label_path, "right-field-guid", "HPOS") == "700",
           "#1488: label layout object distribution should evenly position the middle LBX object");
    expect(visual_object_property(label_path, "middle-field-guid", "VPOS") == "2600" &&
               visual_object_property(label_path, "middle-field-guid", "WIDTH") == "50" &&
               visual_object_property(label_path, "middle-field-guid", "HEIGHT") == "200",
           "#1488: label layout object horizontal distribution should preserve LBX vertical geometry and size");
    expect_contains(distribute_process.stdout_text, "\"isLabel\": true",
                    "#1488: distributed label layout JSON should retain label identity");
    expect_contains(distribute_process.stdout_text, "\"selectedReportObjectAvailable\": true",
                    "#1488: distributed label object JSON should retain selected-object availability");
    expect_contains(distribute_process.stdout_text, "\"left\": 400",
                    "#1488: distributed label object JSON should expose updated left coordinates");
    expect_contains(distribute_process.stdout_text, "\"right\": 450",
                    "#1488: distributed label object JSON should recompute right-edge coordinates");
    expect_contains(distribute_process.stdout_text, "\"sectionObjectIndex\": 1",
                    "#1488: distributed label object JSON should preserve sorted section object order");
    expect_contains(distribute_process.stdout_text, "\"sectionObjectCount\": 3",
                    "#1488: distributed label object JSON should expose containing section object counts");
    expect_contains(distribute_process.stdout_text, "\"containingSectionId\": \"detail_1\"",
                    "#1488: distributed label object JSON should preserve containing section metadata");
    expect_contains(distribute_process.stdout_text, "\"selectedReportObjectSectionAvailable\": true",
                    "#1488: distributed label object JSON should keep selected containing-section availability");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_reorders_label_layout_objects_by_stable_selectors(const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_label_layout_reorder_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path label_path = temp_root / "mailing.lbx";
    write_synthetic_report_table_for_layout_reorder_json(label_path);

    const auto reorder_process = run_process_capture(
        studio_host_path,
        {
            "--path", label_path.string(),
            "--record", "3",
            "--reorder-object",
            "--unique-id", "right-field-guid",
            "--placement", "before",
            "--target-unique-id", "left-field-guid",
            "--json"
        },
        temp_root);

    if (reorder_process.exit_code != 0) {
        std::cerr << "studio host label object reorder stdout:\n" << reorder_process.stdout_text << "\n";
        std::cerr << "studio host label object reorder stderr:\n" << reorder_process.stderr_text << "\n";
        std::cerr << "fixture root: " << temp_root << "\n";
    }

    expect(reorder_process.exit_code == 0,
           "#1489: label layout object reorder should exit successfully");
    expect(visual_object_order(label_path) == "right-field-guid,left-field-guid,middle-field-guid",
           "#1489: label layout object reorder should update physical LBX record order");
    expect_contains(reorder_process.stdout_text, "\"isLabel\": true",
                    "#1489: reordered label layout JSON should retain label identity");
    expect_contains_in_order(
        reorder_process.stdout_text,
        {
            "\"sectionObjectIndex\": 0",
            "\"expression\": \"right.value\"",
            "\"sectionObjectIndex\": 1",
            "\"expression\": \"left.value\"",
            "\"sectionObjectIndex\": 2",
            "\"expression\": \"middle.value\""
        },
        "#1489: label layout JSON should serialize tied-geometry section objects in reordered record order");
    expect_contains(reorder_process.stdout_text, "\"sectionObjectCount\": 3",
                    "#1489: reordered label object JSON should expose containing section object counts");
    expect_contains(reorder_process.stdout_text, "\"containingSectionId\": \"detail_1\"",
                    "#1489: reordered label object JSON should preserve containing section metadata");
    expect_contains(reorder_process.stdout_text, "\"selectedReportObjectSectionAvailable\": true",
                    "#1489: reordered label object JSON should keep selected containing-section availability");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_duplicates_label_layout_objects_by_stable_selectors(const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_label_layout_duplicate_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path label_path = temp_root / "mailing.lbx";
    write_synthetic_report_table_for_layout_reorder_json(label_path);
    const std::size_t before_count = visual_object_count(label_path);

    const auto duplicate_process = run_process_capture(
        studio_host_path,
        {
            "--path", label_path.string(),
            "--duplicate-object",
            "--unique-id", "middle-field-guid",
            "--new-unique-id", "middle-copy-guid",
            "--json"
        },
        temp_root);

    if (duplicate_process.exit_code != 0) {
        std::cerr << "studio host label object duplicate stdout:\n" << duplicate_process.stdout_text << "\n";
        std::cerr << "studio host label object duplicate stderr:\n" << duplicate_process.stderr_text << "\n";
        std::cerr << "fixture root: " << temp_root << "\n";
    }

    expect(duplicate_process.exit_code == 0,
           "#1490: label layout object duplicate should exit successfully");
    expect(visual_object_count(label_path) == before_count + 1U,
           "#1490: label layout object duplicate should append one LBX object record");
    expect(visual_object_exists(label_path, "middle-copy-guid"),
           "#1490: label layout object duplicate should persist replacement unique ids");
    expect(visual_object_order(label_path) == "left-field-guid,middle-field-guid,right-field-guid,middle-copy-guid",
           "#1490: label layout object duplicate should append the copied LBX object after existing layout objects");
    expect_contains(duplicate_process.stdout_text, "\"isLabel\": true",
                    "#1490: duplicated label layout JSON should retain label identity");
    expect_contains_in_order(
        duplicate_process.stdout_text,
        {
            "\"recordIndex\": 5",
            "\"containingSectionId\": \"detail_1\"",
            "\"sectionObjectIndex\": 3",
            "\"sectionObjectCount\": 4",
            "\"expression\": \"middle.value\""
        },
        "#1490: label layout JSON should expose the duplicated object in refreshed section membership");
    expect_contains(duplicate_process.stdout_text, "\"sectionObjectCount\": 4",
                    "#1490: duplicated label object JSON should refresh containing section object counts");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_renames_label_layout_object_identity_by_stable_selectors(
    const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_label_layout_rename_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path label_path = temp_root / "mailing.lbx";
    write_synthetic_report_table_for_layout_reorder_json(label_path);
    const std::size_t before_count = visual_object_count(label_path);

    const auto rename_process = run_process_capture(
        studio_host_path,
        {
            "--path", label_path.string(),
            "--rename-object",
            "--unique-id", "right-field-guid",
            "--new-unique-id", "renamed-right-guid",
            "--json"
        },
        temp_root);

    if (rename_process.exit_code != 0) {
        std::cerr << "studio host label object rename stdout:\n" << rename_process.stdout_text << "\n";
        std::cerr << "studio host label object rename stderr:\n" << rename_process.stderr_text << "\n";
        std::cerr << "fixture root: " << temp_root << "\n";
    }

    expect(rename_process.exit_code == 0,
           "#1491: label layout object rename should exit successfully");
    expect(visual_object_count(label_path) == before_count,
           "#1491: label layout object rename should preserve LBX object count");
    expect(!visual_object_exists(label_path, "right-field-guid"),
           "#1491: label layout object rename should remove the old unique id");
    expect(visual_object_exists(label_path, "renamed-right-guid"),
           "#1491: label layout object rename should persist replacement unique ids");
    expect(visual_object_order(label_path) == "left-field-guid,middle-field-guid,renamed-right-guid",
           "#1491: label layout object rename should preserve physical LBX object order");
    expect_contains(rename_process.stdout_text, "\"isLabel\": true",
                    "#1491: renamed label layout JSON should retain label identity");
    expect_contains_in_order(
        rename_process.stdout_text,
        {
            "\"recordIndex\": 4",
            "\"containingSectionId\": \"detail_1\"",
            "\"sectionObjectIndex\": 2",
            "\"sectionObjectCount\": 3",
            "\"expression\": \"right.value\""
        },
        "#1491: label layout JSON should keep the renamed object in refreshed section membership");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_exposes_selected_summary_label_objects_by_record_selection(
    const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_selected_summary_label_objects_record_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path label_path = temp_root / "selected_summary_object_record.lbx";
    write_synthetic_report_table_for_stable_summary_object_json(label_path);

    const auto object_process = run_process_capture(
        studio_host_path,
        {"--path", label_path.string(), "--record", "3", "--json"},
        temp_root);

    if (object_process.exit_code != 0) {
        std::cerr << "studio host record-selected summary label object stdout:\n"
                  << object_process.stdout_text << "\n";
        std::cerr << "studio host record-selected summary label object stderr:\n"
                  << object_process.stderr_text << "\n";
        std::cerr << "fixture root: " << temp_root << "\n";
    }

    expect(object_process.exit_code == 0,
           "#1977: record-selected summary label object JSON should exit successfully");
    expect_contains(object_process.stdout_text, "\"documentTitle\": \"selected_summary_object_record.lbx\"",
                    "#1977: record-selected summary label object JSON should preserve document titles");
    expect_contains(object_process.stdout_text, "\"isLabel\": true",
                    "#1977: record-selected summary label object JSON should retain label identity");
    expect_contains(object_process.stdout_text, "\"selectedReportObjectAvailable\": true",
                    "#1977: record-selected summary label object selections should advertise selected-object availability");
    expect_contains(object_process.stdout_text, "\"selectedReportSelectionAvailable\": true",
                    "#1977: record-selected summary label object selections should advertise report-selection availability");
    expect_contains(object_process.stdout_text, "\"selectedReportSelectionKind\": \"object\"",
                    "#1977: record-selected summary label object selections should expose object selection kind");
    expect_contains(object_process.stdout_text, "\"previewBoundsAvailable\": true",
                    "#1977: record-selected summary label object JSON should expose live preview availability");
    expect_contains(object_process.stdout_text, "\"previewBoundsLeft\": 0",
                    "#1977: record-selected summary label object JSON should preserve live preview left bounds");
    expect_contains(object_process.stdout_text, "\"previewBoundsTop\": 0",
                    "#1977: record-selected summary label object JSON should preserve live preview top bounds");
    expect_contains(object_process.stdout_text, "\"previewBoundsRight\": 1900",
                    "#1977: record-selected summary label object JSON should preserve live preview right bounds");
    expect_contains(object_process.stdout_text, "\"previewBoundsBottom\": 3900",
                    "#1977: record-selected summary label object JSON should preserve live preview bottom bounds");
    expect_contains(object_process.stdout_text, "\"previewBoundsWidth\": 1900",
                    "#1977: record-selected summary label object JSON should preserve live preview widths");
    expect_contains(object_process.stdout_text, "\"previewBoundsHeight\": 3900",
                    "#1977: record-selected summary label object JSON should preserve live preview heights");
    expect_contains(object_process.stdout_text, "\"deletedPreviewBoundsAvailable\": false",
                    "#1977: record-selected summary label object JSON should not fabricate deleted preview availability");
    expect_contains(object_process.stdout_text, "\"selectedReportSectionAvailable\": false",
                    "#1977: record-selected summary label objects should not advertise selected-section availability");
    expect_contains(object_process.stdout_text, "\"selectedReportSection\": null",
                    "#1977: record-selected summary label objects should serialize null selected sections");
    expect_contains(object_process.stdout_text, "\"selectedReportSettingsAvailable\": false",
                    "#1977: record-selected summary label objects should not advertise selected-settings availability");
    expect_contains(object_process.stdout_text, "\"selectedReportSettings\": null",
                    "#1977: record-selected summary label objects should serialize null selected settings");
    expect_contains(object_process.stdout_text, "\"sectionCount\": 2",
                    "#1977: record-selected summary label object JSON should preserve live section counts");
    expect_contains(object_process.stdout_text, "\"deletedSectionCount\": 0",
                    "#1977: record-selected summary label object JSON should preserve deleted section counts");
    expect_contains(object_process.stdout_text, "\"liveObjectCount\": 1",
                    "#1977: record-selected summary label object JSON should preserve live object counts");
    expect_contains(object_process.stdout_text, "\"deletedObjectCount\": 0",
                    "#1977: record-selected summary label object JSON should preserve deleted object counts");
    expect_contains(object_process.stdout_text, "\"selectedReportObjectSectionAvailable\": true",
                    "#1977: record-selected summary label objects should advertise containing-section availability");
    expect_contains(object_process.stdout_text, "\"selectedReportObjectSection\": {",
                    "#1977: record-selected summary label objects should expose containing-section JSON");
    expect_contains_in_order(
        object_process.stdout_text,
        {
            "\"selectedReportObject\": {",
            "\"recordIndex\": 3",
            "\"deleted\": false",
            "\"containingSectionId\": \"summary_2\"",
            "\"containingSectionRecordIndex\": 2",
            "\"sectionRelativeTop\": 100",
            "\"sectionRelativeBottom\": 350",
            "\"sectionObjectIndex\": 0",
            "\"sectionObjectCount\": 1",
            "\"objectTypeCode\": 5",
            "\"objectKind\": \"label\"",
            "\"expression\": \"\\\"Summary label\\\"\""
        },
        "#1977: record-selected summary label object selections should expose selected-object metadata");
    expect_contains(object_process.stdout_text, "\"left\": 400",
                    "#1977: record-selected summary label object selections should expose selected-object left bounds");
    expect_contains(object_process.stdout_text, "\"top\": 3300",
                    "#1977: record-selected summary label object selections should expose selected-object top bounds");
    expect_contains(object_process.stdout_text, "\"right\": 1900",
                    "#1977: record-selected summary label object selections should expose selected-object right bounds");
    expect_contains(object_process.stdout_text, "\"bottom\": 3550",
                    "#1977: record-selected summary label object selections should expose selected-object bottom bounds");
    expect_contains_in_order(
        object_process.stdout_text,
        {
            "\"selectedReportObjectSection\": {",
            "\"id\": \"summary_2\"",
            "\"bandKind\": \"summary\"",
            "\"recordIndex\": 2",
            "\"deleted\": false",
            "\"sectionIndex\": 1",
            "\"sectionCount\": 2",
            "\"top\": 3200",
            "\"height\": 700",
            "\"bottom\": 3900",
            "\"objectCount\": 1"
        },
        "#1977: record-selected summary label object selections should expose containing summary metadata");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_exposes_selected_deleted_summary_label_objects_by_record_selection(
    const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_selected_deleted_summary_label_objects_record_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path label_path = temp_root / "selected_deleted_summary_object_record.lbx";
    write_synthetic_report_table_for_deleted_summary_object_json(label_path);

    const auto object_process = run_process_capture(
        studio_host_path,
        {"--path", label_path.string(), "--record", "3", "--json"},
        temp_root);

    if (object_process.exit_code != 0) {
        std::cerr << "studio host record-selected deleted summary label object stdout:\n"
                  << object_process.stdout_text << "\n";
        std::cerr << "studio host record-selected deleted summary label object stderr:\n"
                  << object_process.stderr_text << "\n";
        std::cerr << "fixture root: " << temp_root << "\n";
    }

    expect(object_process.exit_code == 0,
           "#1979: record-selected deleted summary label object JSON should exit successfully");
    expect_contains(object_process.stdout_text,
                    "\"documentTitle\": \"selected_deleted_summary_object_record.lbx\"",
                    "#1979: record-selected deleted summary label object JSON should preserve document titles");
    expect_contains(object_process.stdout_text, "\"isLabel\": true",
                    "#1979: record-selected deleted summary label object JSON should retain label identity");
    expect_contains(object_process.stdout_text, "\"selectedReportObjectAvailable\": true",
                    "#1979: record-selected deleted summary label object selections should advertise selected-object availability");
    expect_contains(object_process.stdout_text, "\"selectedReportSelectionAvailable\": true",
                    "#1979: record-selected deleted summary label object selections should advertise report-selection availability");
    expect_contains(object_process.stdout_text, "\"selectedReportSelectionKind\": \"object\"",
                    "#1979: record-selected deleted summary label object selections should expose object selection kind");
    expect_contains(object_process.stdout_text, "\"previewBoundsAvailable\": true",
                    "#1979: record-selected deleted summary label object JSON should expose live preview availability");
    expect_contains(object_process.stdout_text, "\"previewBoundsLeft\": 0",
                    "#1979: record-selected deleted summary label object JSON should preserve live preview left bounds");
    expect_contains(object_process.stdout_text, "\"previewBoundsTop\": 0",
                    "#1979: record-selected deleted summary label object JSON should preserve live preview top bounds");
    expect_contains(object_process.stdout_text, "\"previewBoundsRight\": 0",
                    "#1979: record-selected deleted summary label object JSON should preserve live preview right bounds");
    expect_contains(object_process.stdout_text, "\"previewBoundsBottom\": 3900",
                    "#1979: record-selected deleted summary label object JSON should preserve live preview bottom bounds");
    expect_contains(object_process.stdout_text, "\"previewBoundsWidth\": 0",
                    "#1979: record-selected deleted summary label object JSON should preserve live preview widths");
    expect_contains(object_process.stdout_text, "\"previewBoundsHeight\": 3900",
                    "#1979: record-selected deleted summary label object JSON should preserve live preview heights");
    expect_contains(object_process.stdout_text, "\"deletedPreviewBoundsAvailable\": true",
                    "#1979: record-selected deleted summary label object JSON should expose deleted preview availability");
    expect_contains(object_process.stdout_text, "\"deletedPreviewBoundsLeft\": 400",
                    "#1979: record-selected deleted summary label object JSON should preserve deleted preview left bounds");
    expect_contains(object_process.stdout_text, "\"deletedPreviewBoundsTop\": 3300",
                    "#1979: record-selected deleted summary label object JSON should preserve deleted preview top bounds");
    expect_contains(object_process.stdout_text, "\"deletedPreviewBoundsRight\": 1900",
                    "#1979: record-selected deleted summary label object JSON should preserve deleted preview right bounds");
    expect_contains(object_process.stdout_text, "\"deletedPreviewBoundsBottom\": 3550",
                    "#1979: record-selected deleted summary label object JSON should preserve deleted preview bottom bounds");
    expect_contains(object_process.stdout_text, "\"deletedPreviewBoundsWidth\": 1500",
                    "#1979: record-selected deleted summary label object JSON should preserve deleted preview widths");
    expect_contains(object_process.stdout_text, "\"deletedPreviewBoundsHeight\": 250",
                    "#1979: record-selected deleted summary label object JSON should preserve deleted preview heights");
    expect_contains(object_process.stdout_text, "\"selectedReportSectionAvailable\": false",
                    "#1979: record-selected deleted summary label objects should not advertise selected-section availability");
    expect_contains(object_process.stdout_text, "\"selectedReportSection\": null",
                    "#1979: record-selected deleted summary label objects should serialize null selected sections");
    expect_contains(object_process.stdout_text, "\"selectedReportSettingsAvailable\": false",
                    "#1979: record-selected deleted summary label objects should not advertise selected-settings availability");
    expect_contains(object_process.stdout_text, "\"selectedReportSettings\": null",
                    "#1979: record-selected deleted summary label objects should serialize null selected settings");
    expect_contains(object_process.stdout_text, "\"sectionCount\": 2",
                    "#1979: record-selected deleted summary label object JSON should preserve live section counts");
    expect_contains(object_process.stdout_text, "\"deletedSectionCount\": 0",
                    "#1979: record-selected deleted summary label object JSON should preserve deleted section counts");
    expect_contains(object_process.stdout_text, "\"liveObjectCount\": 0",
                    "#1979: record-selected deleted summary label object JSON should clear live object counts");
    expect_contains(object_process.stdout_text, "\"deletedObjectCount\": 1",
                    "#1979: record-selected deleted summary label object JSON should preserve deleted object counts");
    expect_contains(object_process.stdout_text, "\"selectedReportObjectSectionAvailable\": false",
                    "#1979: record-selected deleted summary label objects should not advertise containing-section availability");
    expect_contains(object_process.stdout_text, "\"selectedReportObjectSection\": null",
                    "#1979: record-selected deleted summary label objects should serialize null containing sections");
    expect_contains_in_order(
        object_process.stdout_text,
        {
            "\"selectedReportObject\": {",
            "\"recordIndex\": 3",
            "\"deleted\": true",
            "\"containingSectionId\": \"\"",
            "\"containingSectionRecordIndex\": null",
            "\"sectionRelativeTop\": 0",
            "\"sectionRelativeBottom\": 0",
            "\"sectionObjectIndex\": null",
            "\"sectionObjectCount\": 0",
            "\"objectTypeCode\": 5",
            "\"objectKind\": \"label\"",
            "\"expression\": \"\\\"Summary label\\\"\""
        },
        "#1979: record-selected deleted summary label object selections should expose selected deleted-object metadata");
    expect_contains(object_process.stdout_text, "\"left\": 400",
                    "#1979: record-selected deleted summary label object selections should expose selected-object left bounds");
    expect_contains(object_process.stdout_text, "\"top\": 3300",
                    "#1979: record-selected deleted summary label object selections should expose selected-object top bounds");
    expect_contains(object_process.stdout_text, "\"right\": 1900",
                    "#1979: record-selected deleted summary label object selections should expose selected-object right bounds");
    expect_contains(object_process.stdout_text, "\"bottom\": 3550",
                    "#1979: record-selected deleted summary label object selections should expose selected-object bottom bounds");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_exposes_selected_label_settings(const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_selected_label_settings_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path label_path = temp_root / "mailing.lbx";
    write_synthetic_report_table_for_layout_json(label_path);

    const auto settings_process = run_process_capture(
        studio_host_path,
        {"--path", label_path.string(), "--record", "0", "--json"},
        temp_root);

    if (settings_process.exit_code != 0) {
        std::cerr << "studio host selected label settings stdout:\n" << settings_process.stdout_text << "\n";
        std::cerr << "studio host selected label settings stderr:\n" << settings_process.stderr_text << "\n";
        std::cerr << "fixture root: " << temp_root << "\n";
    }

    expect(settings_process.exit_code == 0,
           "#1496: selected label settings JSON should exit successfully");
    expect_contains(settings_process.stdout_text, "\"isLabel\": true",
                    "#1496: selected label settings JSON should retain label identity");
    expect_contains(settings_process.stdout_text, "\"selectedReportSettingsAvailable\": true",
                    "#1496: label root selections should advertise selected-settings availability");
    expect_contains(settings_process.stdout_text, "\"selectedReportSelectionAvailable\": true",
                    "#1496: label settings selections should advertise report-selection availability");
    expect_contains(settings_process.stdout_text, "\"selectedReportSelectionKind\": \"settings\"",
                    "#1496: label settings selections should expose settings selection kind");
    expect_contains(settings_process.stdout_text, "\"selectedReportSettings\": [",
                    "#1496: label root selections should expose selected-settings JSON");
    expect_contains(settings_process.stdout_text, "\"previewBoundsAvailable\": true",
                    "#1958: selected label settings JSON should expose live preview availability");
    expect_contains(settings_process.stdout_text, "\"previewBoundsLeft\": 0",
                    "#1958: selected label settings JSON should preserve live preview left bounds");
    expect_contains(settings_process.stdout_text, "\"previewBoundsTop\": 0",
                    "#1958: selected label settings JSON should preserve live preview top bounds");
    expect_contains(settings_process.stdout_text, "\"previewBoundsRight\": 5200",
                    "#1958: selected label settings JSON should preserve live preview right bounds");
    expect_contains(settings_process.stdout_text, "\"previewBoundsBottom\": 8100",
                    "#1958: selected label settings JSON should preserve live preview bottom bounds");
    expect_contains(settings_process.stdout_text, "\"previewBoundsWidth\": 5200",
                    "#1958: selected label settings JSON should preserve live preview widths");
    expect_contains(settings_process.stdout_text, "\"previewBoundsHeight\": 8100",
                    "#1958: selected label settings JSON should preserve live preview heights");
    expect_contains(settings_process.stdout_text, "\"deletedPreviewBoundsAvailable\": true",
                    "#1958: selected label settings JSON should expose deleted preview availability");
    expect_contains(settings_process.stdout_text, "\"deletedPreviewBoundsLeft\": 1000",
                    "#1958: selected label settings JSON should preserve deleted preview left bounds");
    expect_contains(settings_process.stdout_text, "\"deletedPreviewBoundsTop\": 2600",
                    "#1958: selected label settings JSON should preserve deleted preview top bounds");
    expect_contains(settings_process.stdout_text, "\"deletedPreviewBoundsRight\": 2200",
                    "#1958: selected label settings JSON should preserve deleted preview right bounds");
    expect_contains(settings_process.stdout_text, "\"deletedPreviewBoundsBottom\": 2900",
                    "#1958: selected label settings JSON should preserve deleted preview bottom bounds");
    expect_contains(settings_process.stdout_text, "\"deletedPreviewBoundsWidth\": 1200",
                    "#1958: selected label settings JSON should preserve deleted preview widths");
    expect_contains(settings_process.stdout_text, "\"deletedPreviewBoundsHeight\": 300",
                    "#1958: selected label settings JSON should preserve deleted preview heights");
    expect_contains(settings_process.stdout_text, "\"selectedReportSectionAvailable\": false",
                    "#1504: selected label settings should not advertise selected-section availability");
    expect_contains(settings_process.stdout_text, "\"selectedReportSection\": null",
                    "#1504: selected label settings should serialize null selected sections");
    expect_contains(settings_process.stdout_text, "\"selectedReportObjectAvailable\": false",
                    "#1504: selected label settings should not advertise selected-object availability");
    expect_contains(settings_process.stdout_text, "\"selectedReportObject\": null",
                    "#1504: selected label settings should serialize null selected objects");
    expect_contains(settings_process.stdout_text, "\"selectedReportObjectSectionAvailable\": false",
                    "#1504: selected label settings should not advertise containing-object-section availability");
    expect_contains(settings_process.stdout_text, "\"selectedReportObjectSection\": null",
                    "#1504: selected label settings should serialize null containing-object sections");
    expect_contains(settings_process.stdout_text, "\"name\": \"ORIENTATION\", \"recordIndex\": 0, \"fieldIndex\": 2, \"sourceLineIndex\": 0, \"memoBlockNumber\": 1, \"value\": \"0\"",
                    "#1496: selected label settings should expose memo-line setting provenance");
    expect_contains(settings_process.stdout_text, "\"name\": \"PAPERSIZE\", \"recordIndex\": 0, \"fieldIndex\": 2, \"sourceLineIndex\": 1, \"memoBlockNumber\": 1, \"value\": \"1\"",
                    "#1496: selected label settings should expose later memo-line setting provenance");
    expect_contains(settings_process.stdout_text, "\"name\": \"TOPMARGIN\", \"recordIndex\": 0, \"fieldIndex\": 8, \"sourceLineIndex\": null, \"memoBlockNumber\": 0, \"value\": \"10\"",
                    "#1496: selected label settings should expose direct setting provenance");
    expect_contains(settings_process.stdout_text, "\"sectionCount\": 2",
                    "#1496: selected label settings JSON should preserve live section metadata");
    expect_contains(settings_process.stdout_text, "\"unplacedObjectCount\": 1",
                    "#1496: selected label settings JSON should preserve unplaced layout object metadata");
    expect_contains(settings_process.stdout_text, "\"deletedObjectCount\": 1",
                    "#1496: selected label settings JSON should preserve deleted layout object metadata");

    const fs::path deleted_settings_path = temp_root / "deleted_settings.lbx";
    write_synthetic_report_table_for_deleted_settings_json(deleted_settings_path);
    const auto deleted_settings_process = run_process_capture(
        studio_host_path,
        {"--path", deleted_settings_path.string(), "--record", "0", "--json"},
        temp_root);

    if (deleted_settings_process.exit_code != 0) {
        std::cerr << "studio host selected deleted label settings stdout:\n"
                  << deleted_settings_process.stdout_text << "\n";
        std::cerr << "studio host selected deleted label settings stderr:\n"
                  << deleted_settings_process.stderr_text << "\n";
        std::cerr << "fixture root: " << temp_root << "\n";
    }

    expect(deleted_settings_process.exit_code == 0,
           "#1497: selected deleted label settings JSON should exit successfully");
    expect_contains(deleted_settings_process.stdout_text, "\"isLabel\": true",
                    "#1497: selected deleted label settings JSON should retain label identity");
    expect_contains(deleted_settings_process.stdout_text, "\"selectedReportSettingsAvailable\": true",
                    "#1497: deleted label settings selections should advertise selected-settings availability");
    expect_contains(deleted_settings_process.stdout_text, "\"selectedReportSelectionAvailable\": true",
                    "#1497: deleted label settings selections should advertise report-selection availability");
    expect_contains(deleted_settings_process.stdout_text, "\"selectedReportSelectionKind\": \"settings\"",
                    "#1497: deleted label settings selections should expose settings selection kind");
    expect_contains(deleted_settings_process.stdout_text, "\"previewBoundsAvailable\": true",
                    "#1959: selected deleted label settings JSON should expose live preview availability");
    expect_contains(deleted_settings_process.stdout_text, "\"previewBoundsLeft\": 0",
                    "#1959: selected deleted label settings JSON should preserve live preview left bounds");
    expect_contains(deleted_settings_process.stdout_text, "\"previewBoundsTop\": 0",
                    "#1959: selected deleted label settings JSON should preserve live preview top bounds");
    expect_contains(deleted_settings_process.stdout_text, "\"previewBoundsRight\": 5200",
                    "#1959: selected deleted label settings JSON should preserve live preview right bounds");
    expect_contains(deleted_settings_process.stdout_text, "\"previewBoundsBottom\": 8100",
                    "#1959: selected deleted label settings JSON should preserve live preview bottom bounds");
    expect_contains(deleted_settings_process.stdout_text, "\"previewBoundsWidth\": 5200",
                    "#1959: selected deleted label settings JSON should preserve live preview widths");
    expect_contains(deleted_settings_process.stdout_text, "\"previewBoundsHeight\": 8100",
                    "#1959: selected deleted label settings JSON should preserve live preview heights");
    expect_contains(deleted_settings_process.stdout_text, "\"deletedPreviewBoundsAvailable\": true",
                    "#1959: selected deleted label settings JSON should expose deleted preview availability");
    expect_contains(deleted_settings_process.stdout_text, "\"deletedPreviewBoundsLeft\": 1000",
                    "#1959: selected deleted label settings JSON should preserve deleted preview left bounds");
    expect_contains(deleted_settings_process.stdout_text, "\"deletedPreviewBoundsTop\": 2600",
                    "#1959: selected deleted label settings JSON should preserve deleted preview top bounds");
    expect_contains(deleted_settings_process.stdout_text, "\"deletedPreviewBoundsRight\": 2200",
                    "#1959: selected deleted label settings JSON should preserve deleted preview right bounds");
    expect_contains(deleted_settings_process.stdout_text, "\"deletedPreviewBoundsBottom\": 2900",
                    "#1959: selected deleted label settings JSON should preserve deleted preview bottom bounds");
    expect_contains(deleted_settings_process.stdout_text, "\"deletedPreviewBoundsWidth\": 1200",
                    "#1959: selected deleted label settings JSON should preserve deleted preview widths");
    expect_contains(deleted_settings_process.stdout_text, "\"deletedPreviewBoundsHeight\": 300",
                    "#1959: selected deleted label settings JSON should preserve deleted preview heights");
    expect_contains(deleted_settings_process.stdout_text, "\"selectedReportSectionAvailable\": false",
                    "#1505: selected deleted label settings should not advertise selected-section availability");
    expect_contains(deleted_settings_process.stdout_text, "\"selectedReportSection\": null",
                    "#1505: selected deleted label settings should serialize null selected sections");
    expect_contains(deleted_settings_process.stdout_text, "\"selectedReportObjectAvailable\": false",
                    "#1505: selected deleted label settings should not advertise selected-object availability");
    expect_contains(deleted_settings_process.stdout_text, "\"selectedReportObject\": null",
                    "#1505: selected deleted label settings should serialize null selected objects");
    expect_contains(deleted_settings_process.stdout_text, "\"selectedReportObjectSectionAvailable\": false",
                    "#1505: selected deleted label settings should not advertise containing-object-section availability");
    expect_contains(deleted_settings_process.stdout_text, "\"selectedReportObjectSection\": null",
                    "#1505: selected deleted label settings should serialize null containing-object sections");
    expect_contains(deleted_settings_process.stdout_text, "\"settingCount\": 0",
                    "#1497: deleted selected label settings JSON should not expose live settings");
    expect_contains(deleted_settings_process.stdout_text, "\"deletedSettingCount\": 6",
                    "#1497: deleted selected label settings JSON should expose deleted setting counts");
    expect_contains_in_order(
        deleted_settings_process.stdout_text,
        {
            "\"selectedReportSettings\": [",
            "\"name\": \"ORIENTATION\"",
            "\"recordIndex\": 0",
            "\"name\": \"PAPERSIZE\"",
            "\"recordIndex\": 0",
            "\"name\": \"BOTMARGIN\"",
            "\"recordIndex\": 0",
            "\"name\": \"GRIDV\"",
            "\"recordIndex\": 0",
            "\"name\": \"GRIDH\"",
            "\"recordIndex\": 0",
            "\"name\": \"TOPMARGIN\"",
            "\"recordIndex\": 0"
        },
        "#1497: deleted label settings selections should expose selected deleted-setting provenance");
    expect_contains(deleted_settings_process.stdout_text, "\"sectionCount\": 2",
                    "#1497: deleted selected label settings JSON should preserve live section metadata");
    expect_contains(deleted_settings_process.stdout_text, "\"deletedObjectCount\": 1",
                    "#1497: deleted selected label settings JSON should preserve deleted object metadata");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

}  // namespace cf_test_studio_host_json
