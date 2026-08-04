// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.

#include "test_studio_host_json_support.h"

namespace cf_test_studio_host_json {
#if !defined(COPPERFIN_REPORT_LAYOUT_CLASSIFICATIONS_SKIP_HELPERS)
void expect_normalized_classification_preview_bounds(const std::string& text, const std::string& prefix) {
    expect_contains(text, "\"previewBoundsAvailable\": true",
                    prefix + " should preserve live preview availability");
    expect_contains(text, "\"previewBoundsLeft\": 0",
                    prefix + " should preserve live preview left bounds");
    expect_contains(text, "\"previewBoundsTop\": 100",
                    prefix + " should preserve live preview top bounds");
    expect_contains(text, "\"previewBoundsRight\": 360",
                    prefix + " should preserve live preview right bounds");
    expect_contains(text, "\"previewBoundsBottom\": 400",
                    prefix + " should preserve live preview bottom bounds");
    expect_contains(text, "\"previewBoundsWidth\": 360",
                    prefix + " should preserve live preview widths");
    expect_contains(text, "\"previewBoundsHeight\": 300",
                    prefix + " should preserve live preview heights");
    expect_contains(text, "\"deletedPreviewBoundsAvailable\": true",
                    prefix + " should preserve deleted preview availability");
    expect_contains(text, "\"deletedPreviewBoundsLeft\": 0",
                    prefix + " should preserve deleted preview left bounds");
    expect_contains(text, "\"deletedPreviewBoundsTop\": 600",
                    prefix + " should preserve deleted preview top bounds");
    expect_contains(text, "\"deletedPreviewBoundsRight\": 150",
                    prefix + " should preserve deleted preview right bounds");
    expect_contains(text, "\"deletedPreviewBoundsBottom\": 800",
                    prefix + " should preserve deleted preview bottom bounds");
    expect_contains(text, "\"deletedPreviewBoundsWidth\": 150",
                    prefix + " should preserve deleted preview widths");
    expect_contains(text, "\"deletedPreviewBoundsHeight\": 200",
                    prefix + " should preserve deleted preview heights");
}

void expect_unknown_band_preview_bounds(const std::string& text, const std::string& prefix) {
    expect_contains(text, "\"previewBoundsAvailable\": true",
                    prefix + " should preserve live preview availability");
    expect_contains(text, "\"previewBoundsLeft\": 0",
                    prefix + " should preserve live preview left bounds");
    expect_contains(text, "\"previewBoundsTop\": 300",
                    prefix + " should preserve live preview top bounds");
    expect_contains(text, "\"previewBoundsRight\": 0",
                    prefix + " should preserve live preview right bounds");
    expect_contains(text, "\"previewBoundsBottom\": 1000",
                    prefix + " should preserve live preview bottom bounds");
    expect_contains(text, "\"previewBoundsWidth\": 0",
                    prefix + " should preserve live preview widths");
    expect_contains(text, "\"previewBoundsHeight\": 700",
                    prefix + " should preserve live preview heights");
    expect_contains(text, "\"deletedPreviewBoundsAvailable\": true",
                    prefix + " should preserve deleted preview availability");
    expect_contains(text, "\"deletedPreviewBoundsLeft\": 0",
                    prefix + " should preserve deleted preview left bounds");
    expect_contains(text, "\"deletedPreviewBoundsTop\": 1200",
                    prefix + " should preserve deleted preview top bounds");
    expect_contains(text, "\"deletedPreviewBoundsRight\": 0",
                    prefix + " should preserve deleted preview right bounds");
    expect_contains(text, "\"deletedPreviewBoundsBottom\": 1600",
                    prefix + " should preserve deleted preview bottom bounds");
    expect_contains(text, "\"deletedPreviewBoundsWidth\": 0",
                    prefix + " should preserve deleted preview widths");
    expect_contains(text, "\"deletedPreviewBoundsHeight\": 400",
                    prefix + " should preserve deleted preview heights");
}

void write_synthetic_report_table_for_invalid_classification_layout_json(
    const std::filesystem::path& report_path) {
    const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
        {.name = "OBJTYPE", .type = 'C', .length = 48U},
        {.name = "OBJCODE", .type = 'C', .length = 48U},
        {.name = "EXPR", .type = 'M', .length = 4U},
        {.name = "HPOS", .type = 'N', .length = 10U},
        {.name = "VPOS", .type = 'N', .length = 10U},
        {.name = "WIDTH", .type = 'N', .length = 10U},
        {.name = "HEIGHT", .type = 'N', .length = 10U},
        {.name = "UNIQUEID", .type = 'C', .length = 32U}
    };
    const std::string huge_type = "999999999999999999999999999999";
    const std::vector<std::vector<std::string>> records{
        {"1", "53", "ORIENTATION=0", "", "", "", "", ""},
        {"5.bad", "53.bad", "\"Malformed classification\"", "", "0", "", "500", "malformed-class-guid"},
        {huge_type, huge_type, "\"Oversized classification\"", "250", "100", "800", "300",
         "oversized-class-guid"},
        {"8.2.3", "0.trailing", "\"Deleted classification\"", "400", "700", "900", "350",
         "deleted-class-guid"}
    };

    const auto create_result = copperfin::vfp::create_dbf_table_file(report_path.string(), fields, records);
    expect(create_result.ok, "#1718: synthetic report table for invalid layout classifications should be created");
    const auto delete_result = copperfin::vfp::set_record_deleted_flag(report_path.string(), 3U, true);
    expect(delete_result.ok, "#1718: synthetic report table should mark the invalid classification row deleted");
}

void write_synthetic_report_table_for_dot_leading_classification_layout_json(
    const std::filesystem::path& report_path) {
    const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
        {.name = "OBJTYPE", .type = 'C', .length = 48U},
        {.name = "OBJCODE", .type = 'C', .length = 48U},
        {.name = "EXPR", .type = 'M', .length = 4U},
        {.name = "HPOS", .type = 'N', .length = 10U},
        {.name = "VPOS", .type = 'N', .length = 10U},
        {.name = "WIDTH", .type = 'N', .length = 10U},
        {.name = "HEIGHT", .type = 'N', .length = 10U},
        {.name = "UNIQUEID", .type = 'C', .length = 48U}
    };
    const std::vector<std::vector<std::string>> records{
        {"1", "53", "ORIENTATION=0", "", "", "", "", "dot-leading-valid-settings-guid"},
        {".1", ".53", "ORIENTATION=1", "", "", "", "", "dot-leading-root-guid"},
        {".9", ".4", "\"Dot-leading section\"", "", "100", "", "300", "dot-leading-section-guid"},
        {".8", ".0", "\"Dot-leading object\"", "120", "150", "240", "80", "dot-leading-object-guid"},
        {".5", ".1", "\"Deleted dot-leading object\"", "30", "650", "120", "40",
         "dot-leading-deleted-object-guid"}
    };

    const auto create_result = copperfin::vfp::create_dbf_table_file(report_path.string(), fields, records);
    expect(create_result.ok, "#1760: synthetic report table for dot-leading layout classifications should be created");
    const auto delete_result = copperfin::vfp::set_record_deleted_flag(report_path.string(), 4U, true);
    expect(delete_result.ok, "#1760: synthetic report table should mark the dot-leading object deleted");
}

void write_synthetic_report_table_for_negative_classification_layout_json(
    const std::filesystem::path& report_path) {
    const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
        {.name = "OBJTYPE", .type = 'N', .length = 8U},
        {.name = "OBJCODE", .type = 'N', .length = 8U},
        {.name = "EXPR", .type = 'M', .length = 4U},
        {.name = "HPOS", .type = 'N', .length = 10U},
        {.name = "VPOS", .type = 'N', .length = 10U},
        {.name = "WIDTH", .type = 'N', .length = 10U},
        {.name = "HEIGHT", .type = 'N', .length = 10U},
        {.name = "UNIQUEID", .type = 'C', .length = 48U}
    };
    const std::vector<std::vector<std::string>> records{
        {"1", "53", "ORIENTATION=0", "", "", "", "", "negative-valid-settings-guid"},
        {"-1", "53", "ORIENTATION=1", "", "", "", "", "negative-root-guid"},
        {"-9", "4", "\"Negative section\"", "", "100", "", "300", "negative-section-guid"},
        {"-8", "0", "\"Negative object\"", "120", "150", "240", "80", "negative-object-guid"},
        {"-5", "1", "\"Deleted negative object\"", "30", "650", "120", "40", "negative-deleted-object-guid"}
    };

    const auto create_result = copperfin::vfp::create_dbf_table_file(report_path.string(), fields, records);
    expect(create_result.ok, "#1761: synthetic report table for negative layout classifications should be created");
    const auto delete_result = copperfin::vfp::set_record_deleted_flag(report_path.string(), 4U, true);
    expect(delete_result.ok, "#1761: synthetic report table should mark the negative object deleted");
}

void write_synthetic_report_table_for_unsupported_objtype_layout_json(
    const std::filesystem::path& report_path) {
    const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
        {.name = "OBJTYPE", .type = 'N', .length = 8U},
        {.name = "OBJCODE", .type = 'N', .length = 8U},
        {.name = "EXPR", .type = 'M', .length = 4U},
        {.name = "HPOS", .type = 'N', .length = 10U},
        {.name = "VPOS", .type = 'N', .length = 10U},
        {.name = "WIDTH", .type = 'N', .length = 10U},
        {.name = "HEIGHT", .type = 'N', .length = 10U},
        {.name = "UNIQUEID", .type = 'C', .length = 48U}
    };
    const std::vector<std::vector<std::string>> records{
        {"2", "53", "ORIENTATION=0", "", "", "", "", "unsupported-root-like-guid"},
        {"10", "4", "\"Unsupported group\"", "", "100", "", "600", "unsupported-section-like-guid"},
        {"11", "0", "\"Unsupported live object\"", "120", "300", "700", "90",
         "unsupported-object-like-guid"},
        {"12", "5", "\"Unsupported deleted object\"", "260", "620", "500", "120",
         "unsupported-deleted-object-like-guid"}
    };

    const auto create_result = copperfin::vfp::create_dbf_table_file(report_path.string(), fields, records);
    expect(create_result.ok, "#1732: synthetic report table with unsupported OBJTYPE codes should be created");
    const auto delete_result = copperfin::vfp::set_record_deleted_flag(report_path.string(), 3U, true);
    expect(delete_result.ok, "#1732: synthetic report table should mark the unsupported OBJTYPE row deleted");
}

void write_synthetic_report_table_for_fractional_classification_layout_json(
    const std::filesystem::path& report_path) {
    const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
        {.name = "OBJTYPE", .type = 'N', .length = 12U, .decimal_count = 2U},
        {.name = "OBJCODE", .type = 'N', .length = 12U, .decimal_count = 2U},
        {.name = "EXPR", .type = 'M', .length = 4U},
        {.name = "HPOS", .type = 'N', .length = 10U},
        {.name = "VPOS", .type = 'N', .length = 10U},
        {.name = "WIDTH", .type = 'N', .length = 10U},
        {.name = "HEIGHT", .type = 'N', .length = 10U},
        {.name = "UNIQUEID", .type = 'C', .length = 48U}
    };
    const std::vector<std::vector<std::string>> records{
        {"1.90", "53.75", "ORIENTATION=1\nPAPERSIZE=9", "", "", "", "", "fractional-settings-guid"},
        {"9.25", "4.90", "fractional.detail", "", "100", "", "300", "fractional-detail-guid"},
        {"8.80", "0.40", "customer.name", "120", "150", "240", "80", "fractional-field-guid"},
        {"9.50", "7.10", "fractional.page.footer", "", "600", "", "200", "fractional-footer-guid"},
        {"5.99", "1.90", "\"Deleted fractional label\"", "30", "650", "120", "40",
         "fractional-deleted-label-guid"}
    };

    const auto create_result = copperfin::vfp::create_dbf_table_file(report_path.string(), fields, records);
    expect(create_result.ok, "#1758: synthetic report table for fractional layout classifications should be created");
    const auto delete_section_result = copperfin::vfp::set_record_deleted_flag(report_path.string(), 3U, true);
    expect(delete_section_result.ok, "#1758: synthetic report table should mark the fractional section deleted");
    const auto delete_object_result = copperfin::vfp::set_record_deleted_flag(report_path.string(), 4U, true);
    expect(delete_object_result.ok, "#1758: synthetic report table should mark the fractional object deleted");
}

void write_synthetic_report_table_for_trimmed_classification_layout_json(
    const std::filesystem::path& report_path) {
    const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
        {.name = "OBJTYPE", .type = 'C', .length = 16U},
        {.name = "OBJCODE", .type = 'C', .length = 16U},
        {.name = "EXPR", .type = 'M', .length = 4U},
        {.name = "HPOS", .type = 'N', .length = 10U},
        {.name = "VPOS", .type = 'N', .length = 10U},
        {.name = "WIDTH", .type = 'N', .length = 10U},
        {.name = "HEIGHT", .type = 'N', .length = 10U},
        {.name = "UNIQUEID", .type = 'C', .length = 48U}
    };
    const std::vector<std::vector<std::string>> records{
        {" 1 ", " 53 ", "ORIENTATION=0\nPAPERSIZE=1", "", "", "", "", "trimmed-settings-guid"},
        {" 9 ", " 4 ", "trimmed.detail", "", "100", "", "300", "trimmed-detail-guid"},
        {" 8 ", " 0 ", "customer.total", "120", "150", "240", "80", "trimmed-field-guid"},
        {" 9 ", " 7 ", "trimmed.page.footer", "", "600", "", "200", "trimmed-footer-guid"},
        {" 5 ", " 1 ", "\"Deleted trimmed label\"", "30", "650", "120", "40", "trimmed-deleted-label-guid"}
    };

    const auto create_result = copperfin::vfp::create_dbf_table_file(report_path.string(), fields, records);
    expect(create_result.ok, "#1759: synthetic report table for trimmed layout classifications should be created");
    const auto delete_section_result = copperfin::vfp::set_record_deleted_flag(report_path.string(), 3U, true);
    expect(delete_section_result.ok, "#1759: synthetic report table should mark the trimmed section deleted");
    const auto delete_object_result = copperfin::vfp::set_record_deleted_flag(report_path.string(), 4U, true);
    expect(delete_object_result.ok, "#1759: synthetic report table should mark the trimmed object deleted");
}

void write_synthetic_report_table_for_missing_classification_layout_json(
    const std::filesystem::path& report_path) {
    const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
        {.name = "EXPR", .type = 'M', .length = 4U},
        {.name = "HPOS", .type = 'N', .length = 10U},
        {.name = "VPOS", .type = 'N', .length = 10U},
        {.name = "WIDTH", .type = 'N', .length = 10U},
        {.name = "HEIGHT", .type = 'N', .length = 10U},
        {.name = "UNIQUEID", .type = 'C', .length = 32U}
    };
    const std::vector<std::vector<std::string>> records{
        {"ORIENTATION=0", "", "", "", "", ""},
        {"\"Looks like section\"", "", "0", "", "500", "missing-class-section-guid"},
        {"\"Looks like object\"", "125", "200", "300", "80", "missing-class-object-guid"},
        {"\"Looks like deleted object\"", "425", "700", "150", "40", "missing-class-deleted-guid"}
    };

    const auto create_result = copperfin::vfp::create_dbf_table_file(report_path.string(), fields, records);
    expect(create_result.ok, "#1721: synthetic report table without classification fields should be created");
    const auto delete_result = copperfin::vfp::set_record_deleted_flag(report_path.string(), 3U, true);
    expect(delete_result.ok, "#1721: synthetic report table should mark the missing-classification row deleted");
}

void write_synthetic_report_table_for_missing_objtype_layout_json(
    const std::filesystem::path& report_path) {
    const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
        {.name = "OBJCODE", .type = 'N', .length = 8U},
        {.name = "EXPR", .type = 'M', .length = 4U},
        {.name = "HPOS", .type = 'N', .length = 10U},
        {.name = "VPOS", .type = 'N', .length = 10U},
        {.name = "WIDTH", .type = 'N', .length = 10U},
        {.name = "HEIGHT", .type = 'N', .length = 10U},
        {.name = "UNIQUEID", .type = 'C', .length = 48U}
    };
    const std::vector<std::vector<std::string>> records{
        {"53", "ORIENTATION=0", "", "", "", "", "objcode-only-settings-guid"},
        {"4", "\"OBJCODE-only section\"", "", "100", "", "600", "objcode-only-section-guid"},
        {"0", "\"OBJCODE-only object\"", "120", "300", "700", "90", "objcode-only-object-guid"},
        {"5", "\"OBJCODE-only deleted object\"", "260", "620", "500", "120",
         "objcode-only-deleted-object-guid"}
    };

    const auto create_result = copperfin::vfp::create_dbf_table_file(report_path.string(), fields, records);
    expect(create_result.ok, "#1731: synthetic report table without OBJTYPE schema should be created");
    const auto delete_result = copperfin::vfp::set_record_deleted_flag(report_path.string(), 3U, true);
    expect(delete_result.ok, "#1731: synthetic report table should mark the OBJCODE-only row deleted");
}

void write_synthetic_report_table_for_unknown_band_layout_json(
    const std::filesystem::path& report_path) {
    const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
        {.name = "OBJTYPE", .type = 'N', .length = 8U},
        {.name = "OBJCODE", .type = 'N', .length = 8U},
        {.name = "EXPR", .type = 'M', .length = 4U},
        {.name = "VPOS", .type = 'N', .length = 10U},
        {.name = "HEIGHT", .type = 'N', .length = 10U},
        {.name = "UNIQUEID", .type = 'C', .length = 32U}
    };
    const std::vector<std::vector<std::string>> records{
        {"1", "53", "ORIENTATION=0", "", "", ""},
        {"9", "999", "unknown.live", "300", "700", "unknown-live-band-guid"},
        {"9", "1234", "unknown.deleted", "1200", "400", "unknown-deleted-band-guid"}
    };

    const auto create_result = copperfin::vfp::create_dbf_table_file(report_path.string(), fields, records);
    expect(create_result.ok, "#1722: synthetic report table with unknown band codes should be created");
    const auto delete_result = copperfin::vfp::set_record_deleted_flag(report_path.string(), 2U, true);
    expect(delete_result.ok, "#1722: synthetic report table should mark the unknown deleted band");
}

void write_synthetic_report_table_for_invalid_direct_page_setup_layout_json(
    const std::filesystem::path& report_path) {
    const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
        {.name = "OBJTYPE", .type = 'N', .length = 8U},
        {.name = "OBJCODE", .type = 'N', .length = 8U},
        {.name = "ORIENTATIO", .type = 'C', .length = 24U},
        {.name = "PAPERSIZE", .type = 'C', .length = 24U},
        {.name = "TOPMARGIN", .type = 'C', .length = 24U},
        {.name = "UNIQUEID", .type = 'C', .length = 48U}
    };
    const std::vector<std::vector<std::string>> records{
        {"1", "53", "2.bad", "9.1junk", "100.2.3", "invalid-direct-live-settings-guid"},
        {"1", "53", "3.bad", "10.1junk", "200.2.3",
         "invalid-direct-deleted-settings-guid"}
    };

    const auto create_result = copperfin::vfp::create_dbf_table_file(report_path.string(), fields, records);
    expect(create_result.ok, "#1733: synthetic report table with invalid direct page setup fields should be created");
    const auto delete_result = copperfin::vfp::set_record_deleted_flag(report_path.string(), 1U, true);
    expect(delete_result.ok, "#1733: synthetic report table should mark invalid direct settings deleted");
}
#endif

#if !defined(COPPERFIN_REPORT_LAYOUT_CLASSIFICATIONS_SKIP_HOST_SMOKE)
void test_studio_host_json_ignores_invalid_report_layout_classifications(
    const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_invalid_report_layout_classification_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const auto expect_no_report_selection = [](const std::string& stdout_text, const std::string& message_prefix) {
        expect_contains(stdout_text, "\"selectedReportSelectionAvailable\": false",
                        message_prefix + " should not expose report-selection availability");
        expect_contains(stdout_text, "\"selectedReportSelectionKind\": \"none\"",
                        message_prefix + " should expose explicit no-selection kind");
        expect_contains(stdout_text, "\"selectedReportObjectAvailable\": false",
                        message_prefix + " should not expose selected-object availability");
        expect_contains(stdout_text, "\"selectedReportObject\": null",
                        message_prefix + " should serialize null selected objects");
        expect_contains(stdout_text, "\"selectedReportObjectSectionAvailable\": false",
                        message_prefix + " should not expose containing-section availability");
        expect_contains(stdout_text, "\"selectedReportObjectSection\": null",
                        message_prefix + " should serialize null containing sections");
        expect_contains(stdout_text, "\"selectedReportSectionAvailable\": false",
                        message_prefix + " should not expose selected-section availability");
        expect_contains(stdout_text, "\"selectedReportSection\": null",
                        message_prefix + " should serialize null selected sections");
        expect_contains(stdout_text, "\"selectedReportSettingsAvailable\": false",
                        message_prefix + " should not expose selected-settings availability");
        expect_contains(stdout_text, "\"selectedReportSettings\": null",
                        message_prefix + " should serialize null selected settings");
    };

    const auto run_invalid_classification_layout = [&](const fs::path& asset_path,
                                                       const std::string& title,
                                                       const std::string& label) {
        write_synthetic_report_table_for_invalid_classification_layout_json(asset_path);

        const auto summary_process = run_process_capture(
            studio_host_path,
            {"--path", asset_path.string(), "--json"},
            temp_root);

        if (summary_process.exit_code != 0) {
            std::cerr << "studio host " << label << " invalid classification summary stdout:\n"
                      << summary_process.stdout_text << "\n";
            std::cerr << "studio host " << label << " invalid classification summary stderr:\n"
                      << summary_process.stderr_text << "\n";
            std::cerr << "fixture root: " << temp_root << "\n";
        }

        expect(summary_process.exit_code == 0,
               "#1718: invalid report/label layout classifications should keep inspection non-failing");
        expect_contains(summary_process.stdout_text, "\"documentTitle\": \"" + title + "\"",
                        "#1718: invalid layout classifications should preserve document titles");
        if (asset_path.extension() == ".lbx") {
            expect_contains(summary_process.stdout_text, "\"isLabel\": true",
                            "#1718: invalid classification label layouts should retain label identity");
        }
        expect_contains(summary_process.stdout_text, "\"pageSetupAvailable\": true",
                        "#1718: invalid classification rows should preserve root report settings");
        expect_contains(summary_process.stdout_text, "\"previewBoundsAvailable\": false",
                        "#1718: invalid classification rows should not create live preview bounds");
        expect_contains(summary_process.stdout_text, "\"deletedPreviewBoundsAvailable\": false",
                        "#1718: invalid classification rows should not create deleted preview bounds");
        expect_empty_report_layout_preview_bounds(
            summary_process.stdout_text,
            "#2356: invalid classification summary JSON");
        expect_contains(summary_process.stdout_text, "\"liveObjectCount\": 0",
                        "#1718: invalid classification rows should not create live layout objects");
        expect_contains(summary_process.stdout_text, "\"deletedObjectCount\": 0",
                        "#1718: invalid classification rows should not create deleted layout objects");
        expect_contains(summary_process.stdout_text, "\"sectionCount\": 0",
                        "#1718: invalid classification rows should not create live sections");
        expect_contains(summary_process.stdout_text, "\"deletedSectionCount\": 0",
                        "#1718: invalid classification rows should not create deleted sections");
        expect_contains(summary_process.stdout_text, "\"settingCount\": 1",
                        "#1718: invalid classification rows should preserve live root settings");

        for (const auto record_index : {1, 2, 3}) {
            const auto selected_process = run_process_capture(
                studio_host_path,
                {"--path", asset_path.string(), "--record", std::to_string(record_index), "--json"},
                temp_root);

            expect(selected_process.exit_code == 0,
                   "#1718: invalid classification record selection should keep inspection non-failing");
            expect_no_report_selection(
                selected_process.stdout_text,
                "#1718: invalid classification record " + std::to_string(record_index));
            expect_empty_report_layout_preview_bounds(
                selected_process.stdout_text,
                "#2356: selected invalid classification record " + std::to_string(record_index));
        }
    };

    run_invalid_classification_layout(temp_root / "invalid_classifications.frx",
                                      "invalid_classifications.frx",
                                      "report");
    run_invalid_classification_layout(temp_root / "invalid_classifications.lbx",
                                      "invalid_classifications.lbx",
                                      "label");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_ignores_dot_leading_report_layout_classifications(
    const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_dot_leading_report_layout_classification_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const auto expect_no_report_selection = [](const std::string& stdout_text, const std::string& message_prefix) {
        expect_contains(stdout_text, "\"selectedReportSelectionAvailable\": false",
                        message_prefix + " should not expose report-selection availability");
        expect_contains(stdout_text, "\"selectedReportSelectionKind\": \"none\"",
                        message_prefix + " should expose explicit no-selection kind");
        expect_contains(stdout_text, "\"selectedReportObjectAvailable\": false",
                        message_prefix + " should not expose selected-object availability");
        expect_contains(stdout_text, "\"selectedReportObject\": null",
                        message_prefix + " should serialize null selected objects");
        expect_contains(stdout_text, "\"selectedReportObjectSectionAvailable\": false",
                        message_prefix + " should not expose containing-section availability");
        expect_contains(stdout_text, "\"selectedReportObjectSection\": null",
                        message_prefix + " should serialize null containing sections");
        expect_contains(stdout_text, "\"selectedReportSectionAvailable\": false",
                        message_prefix + " should not expose selected-section availability");
        expect_contains(stdout_text, "\"selectedReportSection\": null",
                        message_prefix + " should serialize null selected sections");
        expect_contains(stdout_text, "\"selectedReportSettingsAvailable\": false",
                        message_prefix + " should not expose selected-settings availability");
        expect_contains(stdout_text, "\"selectedReportSettings\": null",
                        message_prefix + " should serialize null selected settings");
    };

    const auto run_dot_leading_classification_layout = [&](const fs::path& asset_path,
                                                           const std::string& title,
                                                           const std::string& label) {
        write_synthetic_report_table_for_dot_leading_classification_layout_json(asset_path);

        const auto summary_process = run_process_capture(
            studio_host_path,
            {"--path", asset_path.string(), "--json"},
            temp_root);

        if (summary_process.exit_code != 0) {
            std::cerr << "studio host " << label << " dot-leading classification summary stdout:\n"
                      << summary_process.stdout_text << "\n";
            std::cerr << "studio host " << label << " dot-leading classification summary stderr:\n"
                      << summary_process.stderr_text << "\n";
            std::cerr << "fixture root: " << temp_root << "\n";
        }

        expect(summary_process.exit_code == 0,
               "#1760: dot-leading report/label layout classifications should keep inspection non-failing");
        expect_contains(summary_process.stdout_text, "\"documentTitle\": \"" + title + "\"",
                        "#1760: dot-leading layout classifications should preserve document titles");
        if (asset_path.extension() == ".lbx") {
            expect_contains(summary_process.stdout_text, "\"isLabel\": true",
                            "#1760: dot-leading classification label layouts should retain label identity");
        }
        expect_contains(summary_process.stdout_text, "\"pageSetupAvailable\": true",
                        "#1760: dot-leading classification rows should preserve valid root settings");
        expect_contains(summary_process.stdout_text, "\"orientationCode\": 0",
                        "#1760: dot-leading root-like rows should not override valid settings");
        expect_contains(summary_process.stdout_text, "\"settingCount\": 1",
                        "#1760: dot-leading root-like rows should not create extra live root settings");
        expect_contains(summary_process.stdout_text, "\"deletedSettingCount\": 0",
                        "#1760: dot-leading root-like rows should not create deleted root settings");
        expect_contains(summary_process.stdout_text, "\"previewBoundsAvailable\": false",
                        "#1760: dot-leading classification rows should not create live preview bounds");
        expect_contains(summary_process.stdout_text, "\"deletedPreviewBoundsAvailable\": false",
                        "#1760: dot-leading classification rows should not create deleted preview bounds");
        expect_empty_report_layout_preview_bounds(
            summary_process.stdout_text,
            "#2357: dot-leading classification summary JSON");
        expect_contains(summary_process.stdout_text, "\"sectionCount\": 0",
                        "#1760: dot-leading classification rows should not create live sections");
        expect_contains(summary_process.stdout_text, "\"deletedSectionCount\": 0",
                        "#1760: dot-leading classification rows should not create deleted sections");
        expect_contains(summary_process.stdout_text, "\"liveObjectCount\": 0",
                        "#1760: dot-leading classification rows should not create live layout objects");
        expect_contains(summary_process.stdout_text, "\"deletedObjectCount\": 0",
                        "#1760: dot-leading classification rows should not create deleted layout objects");

        for (const auto record_index : {1, 2, 3, 4}) {
            const auto selected_process = run_process_capture(
                studio_host_path,
                {"--path", asset_path.string(), "--record", std::to_string(record_index), "--json"},
                temp_root);

            expect(selected_process.exit_code == 0,
                   "#1760: dot-leading classification record selection should keep inspection non-failing");
            expect_no_report_selection(
                selected_process.stdout_text,
                "#1760: dot-leading classification record " + std::to_string(record_index));
            expect_empty_report_layout_preview_bounds(
                selected_process.stdout_text,
                "#2357: selected dot-leading classification record " + std::to_string(record_index));
        }
    };

    run_dot_leading_classification_layout(temp_root / "dot_leading_classifications.frx",
                                          "dot_leading_classifications.frx",
                                          "report");
    run_dot_leading_classification_layout(temp_root / "dot_leading_classifications.lbx",
                                          "dot_leading_classifications.lbx",
                                          "label");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_ignores_negative_report_layout_classifications(
    const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_negative_report_layout_classification_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const auto expect_no_report_selection = [](const std::string& stdout_text, const std::string& message_prefix) {
        expect_contains(stdout_text, "\"selectedReportSelectionAvailable\": false",
                        message_prefix + " should not expose report-selection availability");
        expect_contains(stdout_text, "\"selectedReportSelectionKind\": \"none\"",
                        message_prefix + " should expose explicit no-selection kind");
        expect_contains(stdout_text, "\"selectedReportObjectAvailable\": false",
                        message_prefix + " should not expose selected-object availability");
        expect_contains(stdout_text, "\"selectedReportObject\": null",
                        message_prefix + " should serialize null selected objects");
        expect_contains(stdout_text, "\"selectedReportObjectSectionAvailable\": false",
                        message_prefix + " should not expose containing-section availability");
        expect_contains(stdout_text, "\"selectedReportObjectSection\": null",
                        message_prefix + " should serialize null containing sections");
        expect_contains(stdout_text, "\"selectedReportSectionAvailable\": false",
                        message_prefix + " should not expose selected-section availability");
        expect_contains(stdout_text, "\"selectedReportSection\": null",
                        message_prefix + " should serialize null selected sections");
        expect_contains(stdout_text, "\"selectedReportSettingsAvailable\": false",
                        message_prefix + " should not expose selected-settings availability");
        expect_contains(stdout_text, "\"selectedReportSettings\": null",
                        message_prefix + " should serialize null selected settings");
    };

    const auto run_negative_classification_layout = [&](const fs::path& asset_path,
                                                        const std::string& title,
                                                        const std::string& label) {
        write_synthetic_report_table_for_negative_classification_layout_json(asset_path);

        const auto summary_process = run_process_capture(
            studio_host_path,
            {"--path", asset_path.string(), "--json"},
            temp_root);

        if (summary_process.exit_code != 0) {
            std::cerr << "studio host " << label << " negative classification summary stdout:\n"
                      << summary_process.stdout_text << "\n";
            std::cerr << "studio host " << label << " negative classification summary stderr:\n"
                      << summary_process.stderr_text << "\n";
            std::cerr << "fixture root: " << temp_root << "\n";
        }

        expect(summary_process.exit_code == 0,
               "#1761: negative report/label layout classifications should keep inspection non-failing");
        expect_contains(summary_process.stdout_text, "\"documentTitle\": \"" + title + "\"",
                        "#1761: negative layout classifications should preserve document titles");
        if (asset_path.extension() == ".lbx") {
            expect_contains(summary_process.stdout_text, "\"isLabel\": true",
                            "#1761: negative classification label layouts should retain label identity");
        }
        expect_contains(summary_process.stdout_text, "\"pageSetupAvailable\": true",
                        "#1761: negative classification rows should preserve valid root settings");
        expect_contains(summary_process.stdout_text, "\"orientationCode\": 0",
                        "#1761: negative root-like rows should not override valid settings");
        expect_contains(summary_process.stdout_text, "\"settingCount\": 1",
                        "#1761: negative root-like rows should not create extra live root settings");
        expect_contains(summary_process.stdout_text, "\"deletedSettingCount\": 0",
                        "#1761: negative root-like rows should not create deleted root settings");
        expect_contains(summary_process.stdout_text, "\"previewBoundsAvailable\": false",
                        "#1761: negative classification rows should not create live preview bounds");
        expect_contains(summary_process.stdout_text, "\"deletedPreviewBoundsAvailable\": false",
                        "#1761: negative classification rows should not create deleted preview bounds");
        expect_empty_report_layout_preview_bounds(
            summary_process.stdout_text,
            "#2358: negative classification summary JSON");
        expect_contains(summary_process.stdout_text, "\"sectionCount\": 0",
                        "#1761: negative classification rows should not create live sections");
        expect_contains(summary_process.stdout_text, "\"deletedSectionCount\": 0",
                        "#1761: negative classification rows should not create deleted sections");
        expect_contains(summary_process.stdout_text, "\"liveObjectCount\": 0",
                        "#1761: negative classification rows should not create live layout objects");
        expect_contains(summary_process.stdout_text, "\"deletedObjectCount\": 0",
                        "#1761: negative classification rows should not create deleted layout objects");

        for (const auto record_index : {1, 2, 3, 4}) {
            const auto selected_process = run_process_capture(
                studio_host_path,
                {"--path", asset_path.string(), "--record", std::to_string(record_index), "--json"},
                temp_root);

            expect(selected_process.exit_code == 0,
                   "#1761: negative classification record selection should keep inspection non-failing");
            expect_no_report_selection(
                selected_process.stdout_text,
                "#1761: negative classification record " + std::to_string(record_index));
            expect_empty_report_layout_preview_bounds(
                selected_process.stdout_text,
                "#2358: selected negative classification record " + std::to_string(record_index));
        }
    };

    run_negative_classification_layout(temp_root / "negative_classifications.frx",
                                       "negative_classifications.frx",
                                       "report");
    run_negative_classification_layout(temp_root / "negative_classifications.lbx",
                                       "negative_classifications.lbx",
                                       "label");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_ignores_unsupported_report_layout_objtype_codes(
    const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_unsupported_report_layout_objtype_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const auto expect_no_report_selection = [](const std::string& stdout_text, const std::string& message_prefix) {
        expect_contains(stdout_text, "\"selectedReportSelectionAvailable\": false",
                        message_prefix + " should not expose report-selection availability");
        expect_contains(stdout_text, "\"selectedReportSelectionKind\": \"none\"",
                        message_prefix + " should expose explicit no-selection kind");
        expect_contains(stdout_text, "\"selectedReportObjectAvailable\": false",
                        message_prefix + " should not expose selected-object availability");
        expect_contains(stdout_text, "\"selectedReportObject\": null",
                        message_prefix + " should serialize null selected objects");
        expect_contains(stdout_text, "\"selectedReportObjectSectionAvailable\": false",
                        message_prefix + " should not expose containing-section availability");
        expect_contains(stdout_text, "\"selectedReportObjectSection\": null",
                        message_prefix + " should serialize null containing sections");
        expect_contains(stdout_text, "\"selectedReportSectionAvailable\": false",
                        message_prefix + " should not expose selected-section availability");
        expect_contains(stdout_text, "\"selectedReportSection\": null",
                        message_prefix + " should serialize null selected sections");
        expect_contains(stdout_text, "\"selectedReportSettingsAvailable\": false",
                        message_prefix + " should not expose selected-settings availability");
        expect_contains(stdout_text, "\"selectedReportSettings\": null",
                        message_prefix + " should serialize null selected settings");
        expect_empty_report_layout_preview_bounds(
            stdout_text,
            "#2315: " + message_prefix + " selected unsupported OBJTYPE JSON");
    };

    const auto run_unsupported_objtype_layout = [&](const fs::path& asset_path,
                                                    const std::string& title,
                                                    const std::string& label) {
        write_synthetic_report_table_for_unsupported_objtype_layout_json(asset_path);

        const auto summary_process = run_process_capture(
            studio_host_path,
            {"--path", asset_path.string(), "--json"},
            temp_root);

        if (summary_process.exit_code != 0) {
            std::cerr << "studio host " << label << " unsupported OBJTYPE summary stdout:\n"
                      << summary_process.stdout_text << "\n";
            std::cerr << "studio host " << label << " unsupported OBJTYPE summary stderr:\n"
                      << summary_process.stderr_text << "\n";
            std::cerr << "fixture root: " << temp_root << "\n";
        }

        expect(summary_process.exit_code == 0,
               "#1732: unsupported report/label layout OBJTYPE codes should keep inspection non-failing");
        expect_contains(summary_process.stdout_text, "\"documentTitle\": \"" + title + "\"",
                        "#1732: unsupported OBJTYPE layouts should preserve document titles");
        if (asset_path.extension() == ".lbx") {
            expect_contains(summary_process.stdout_text, "\"isLabel\": true",
                            "#1732: unsupported OBJTYPE label layouts should retain label identity");
        }
        expect_contains(summary_process.stdout_text, "\"previewBoundsAvailable\": false",
                        "#1732: unsupported OBJTYPE rows should not create live preview bounds");
        expect_contains(summary_process.stdout_text, "\"deletedPreviewBoundsAvailable\": false",
                        "#1732: unsupported OBJTYPE rows should not create deleted preview bounds");
        expect_empty_report_layout_preview_bounds(
            summary_process.stdout_text,
            "#2345: unsupported OBJTYPE summary JSON");
        expect_contains(summary_process.stdout_text, "\"pageSetupAvailable\": false",
                        "#1732: unsupported OBJTYPE rows should not infer root settings");
        expect_contains(summary_process.stdout_text, "\"liveObjectCount\": 0",
                        "#1732: unsupported OBJTYPE rows should not create live layout objects");
        expect_contains(summary_process.stdout_text, "\"deletedObjectCount\": 0",
                        "#1732: unsupported OBJTYPE rows should not create deleted layout objects");
        expect_contains(summary_process.stdout_text, "\"sectionCount\": 0",
                        "#1732: unsupported OBJTYPE rows should not create live sections");
        expect_contains(summary_process.stdout_text, "\"deletedSectionCount\": 0",
                        "#1732: unsupported OBJTYPE rows should not create deleted sections");
        expect_contains(summary_process.stdout_text, "\"settingCount\": 0",
                        "#1732: unsupported OBJTYPE rows should not create live root settings");
        expect_contains(summary_process.stdout_text, "\"deletedSettingCount\": 0",
                        "#1732: unsupported OBJTYPE rows should not create deleted root settings");

        for (const auto record_index : {0, 1, 2, 3}) {
            const auto selected_process = run_process_capture(
                studio_host_path,
                {"--path", asset_path.string(), "--record", std::to_string(record_index), "--json"},
                temp_root);

            expect(selected_process.exit_code == 0,
                   "#1732: unsupported OBJTYPE record selection should keep inspection non-failing");
            expect_no_report_selection(
                selected_process.stdout_text,
                "#1732: unsupported OBJTYPE record " + std::to_string(record_index));
        }
    };

    run_unsupported_objtype_layout(temp_root / "unsupported_objtype.frx",
                                   "unsupported_objtype.frx",
                                   "report");
    run_unsupported_objtype_layout(temp_root / "unsupported_objtype.lbx",
                                   "unsupported_objtype.lbx",
                                   "label");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_uses_integer_portions_for_fractional_report_layout_classifications(
    const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_fractional_report_layout_classification_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    ScopedDefaultLocaleCatalogEnvironment default_locale_environment;

    const auto run_fractional_classification_layout = [&](const fs::path& asset_path,
                                                          const std::string& title,
                                                          const std::string& label) {
        write_synthetic_report_table_for_fractional_classification_layout_json(asset_path);

        const auto summary_process = run_process_capture(
            studio_host_path,
            {"--path", asset_path.string(), "--json"},
            temp_root);

        if (summary_process.exit_code != 0) {
            std::cerr << "studio host " << label << " fractional classification summary stdout:\n"
                      << summary_process.stdout_text << "\n";
            std::cerr << "studio host " << label << " fractional classification summary stderr:\n"
                      << summary_process.stderr_text << "\n";
            std::cerr << "fixture root: " << temp_root << "\n";
        }

        expect(summary_process.exit_code == 0,
               "#1758: fractional report/label layout classifications should keep inspection non-failing");
        expect_contains(summary_process.stdout_text, "\"documentTitle\": \"" + title + "\"",
                        "#1758: fractional layout classifications should preserve document titles");
        if (asset_path.extension() == ".lbx") {
            expect_contains(summary_process.stdout_text, "\"isLabel\": true",
                            "#1758: fractional classification label layouts should retain label identity");
        }
        expect_contains(summary_process.stdout_text, "\"pageSetupAvailable\": true",
                        "#1758: fractional root OBJTYPE should expose settings summaries");
        expect_contains(summary_process.stdout_text, "\"orientationCode\": 1",
                        "#1758: fractional root classifications should preserve settings values");
        expect_contains(summary_process.stdout_text, "\"paperSizeCode\": 9",
                        "#1758: fractional root classifications should preserve later settings values");
        expect_contains(summary_process.stdout_text, "\"settingCount\": 2",
                        "#1758: fractional root classifications should preserve live root settings");
        expect_normalized_classification_preview_bounds(
            summary_process.stdout_text,
            "#2312: fractional classification summary JSON");
        expect_contains(summary_process.stdout_text, "\"sectionCount\": 1",
                        "#1758: fractional band classifications should create live sections");
        expect_contains(summary_process.stdout_text, "\"deletedSectionCount\": 1",
                        "#1758: fractional deleted band classifications should create deleted sections");
        expect_contains(summary_process.stdout_text, "\"liveObjectCount\": 1",
                        "#1758: fractional object classifications should create live layout objects");
        expect_contains(summary_process.stdout_text, "\"placedObjectCount\": 1",
                        "#1758: fractional object classifications should place live objects in sections");
        expect_contains(summary_process.stdout_text, "\"deletedObjectCount\": 1",
                        "#1758: fractional deleted object classifications should create deleted objects");
        expect_contains(summary_process.stdout_text, "\"deletedPlacedObjectCount\": 1",
                        "#1758: fractional deleted object classifications should place deleted objects in bands");
        expect_contains(summary_process.stdout_text, "{\"kind\": \"field\", \"count\": 1}",
                        "#1758: fractional live object OBJTYPE should use integer portions for kind counts");
        expect_contains(summary_process.stdout_text, "{\"kind\": \"label\", \"count\": 1}",
                        "#1758: fractional deleted object OBJTYPE should use integer portions for kind counts");
        expect_contains(summary_process.stdout_text, "{\"kind\": \"detail\", \"count\": 1}",
                        "#1758: fractional live band OBJCODE should use integer portions for kind counts");
        expect_contains(summary_process.stdout_text, "{\"kind\": \"page_footer\", \"count\": 1}",
                        "#1758: fractional deleted band OBJCODE should use integer portions for kind counts");

        const auto settings_process = run_process_capture(
            studio_host_path,
            {"--path", asset_path.string(), "--record", "0", "--json"},
            temp_root);
        expect(settings_process.exit_code == 0,
               "#1758: fractional root classification selection should keep inspection non-failing");
        expect_contains(settings_process.stdout_text, "\"selectedReportSettingsAvailable\": true",
                        "#1758: fractional root classification should resolve selected settings");
        expect_contains(settings_process.stdout_text, "\"selectedReportSelectionKind\": \"settings\"",
                        "#1758: fractional root classification should expose settings selection kind");
        expect_contains_in_order(
            settings_process.stdout_text,
            {
                "\"selectedReportSettings\": [",
                "\"name\": \"ORIENTATION\", \"recordIndex\": 0",
                "\"value\": \"1\"",
                "\"name\": \"PAPERSIZE\", \"recordIndex\": 0",
                "\"value\": \"9\""
            },
            "#1758: fractional root classification should preserve selected settings provenance");
        expect_normalized_classification_preview_bounds(
            settings_process.stdout_text,
            "#2359: selected fractional settings classification JSON");

        const auto section_process = run_process_capture(
            studio_host_path,
            {"--path", asset_path.string(), "--record", "1", "--json"},
            temp_root);
        expect(section_process.exit_code == 0,
               "#1758: fractional live section classification selection should keep inspection non-failing");
        expect_contains(section_process.stdout_text, "\"selectedReportSectionAvailable\": true",
                        "#1758: fractional band classification should resolve selected sections");
        expect_contains(section_process.stdout_text, "\"selectedReportSelectionKind\": \"section\"",
                        "#1758: fractional band classification should expose section selection kind");
        expect_contains_in_order(
            section_process.stdout_text,
            {
                "\"selectedReportSection\": {",
                "\"recordIndex\": 1",
                "\"deleted\": false"
            },
            "#1758: fractional live section classification should use integer portions");
        expect_contains(section_process.stdout_text, "\"title\": \"Detail\"",
                        "#1758: fractional live section classification should preserve integer band titles");
        expect_contains(section_process.stdout_text, "\"bandKind\": \"detail\"",
                        "#1758: fractional live section classification should preserve integer band kinds");
        expect_contains(section_process.stdout_text, "\"objectCode\": 4",
                        "#1758: fractional live section classification should preserve integer band codes");
        expect_normalized_classification_preview_bounds(
            section_process.stdout_text,
            "#2312: selected fractional live section classification JSON");

        const auto object_process = run_process_capture(
            studio_host_path,
            {"--path", asset_path.string(), "--record", "2", "--json"},
            temp_root);
        expect(object_process.exit_code == 0,
               "#1758: fractional live object classification selection should keep inspection non-failing");
        expect_contains(object_process.stdout_text, "\"selectedReportObjectAvailable\": true",
                        "#1758: fractional object classification should resolve selected objects");
        expect_contains(object_process.stdout_text, "\"selectedReportSelectionKind\": \"object\"",
                        "#1758: fractional object classification should expose object selection kind");
        expect_contains(object_process.stdout_text, "\"selectedReportObjectSectionAvailable\": true",
                        "#1758: fractional object classification should preserve containing sections");
        expect_contains_in_order(
            object_process.stdout_text,
            {
                "\"selectedReportObject\": {",
                "\"recordIndex\": 2",
                "\"deleted\": false"
            },
            "#1758: fractional live object classification should use integer portions");
        expect_contains(object_process.stdout_text, "\"objectTypeCode\": 8",
                        "#1758: fractional live object classification should preserve integer object type codes");
        expect_contains(object_process.stdout_text, "\"objectKind\": \"field\"",
                        "#1758: fractional live object classification should preserve integer object kinds");
        expect_contains(object_process.stdout_text, "\"objectCode\": 0",
                        "#1758: fractional live object classification should preserve integer object codes");
        expect_contains(object_process.stdout_text, "\"containingSectionRecordIndex\": 1",
                        "#1758: fractional live object classification should preserve containing sections");
        expect_normalized_classification_preview_bounds(
            object_process.stdout_text,
            "#2312: selected fractional live object classification JSON");

        const auto deleted_section_process = run_process_capture(
            studio_host_path,
            {"--path", asset_path.string(), "--record", "3", "--json"},
            temp_root);
        expect(deleted_section_process.exit_code == 0,
               "#1758: fractional deleted section classification selection should keep inspection non-failing");
        expect_contains(deleted_section_process.stdout_text, "\"selectedReportSectionAvailable\": true",
                        "#1758: fractional deleted band classification should resolve selected sections");
        expect_contains_in_order(
            deleted_section_process.stdout_text,
            {
                "\"selectedReportSection\": {",
                "\"recordIndex\": 3",
                "\"deleted\": true"
            },
            "#1758: fractional deleted section classification should use integer portions");
        expect_contains(deleted_section_process.stdout_text, "\"title\": \"Page Footer\"",
                        "#1758: fractional deleted section classification should preserve integer band titles");
        expect_contains(deleted_section_process.stdout_text, "\"bandKind\": \"page_footer\"",
                        "#1758: fractional deleted section classification should preserve integer band kinds");
        expect_contains(deleted_section_process.stdout_text, "\"objectCode\": 7",
                        "#1758: fractional deleted section classification should preserve integer band codes");
        expect_normalized_classification_preview_bounds(
            deleted_section_process.stdout_text,
            "#2312: selected fractional deleted section classification JSON");

        const auto deleted_object_process = run_process_capture(
            studio_host_path,
            {"--path", asset_path.string(), "--record", "4", "--json"},
            temp_root);
        expect(deleted_object_process.exit_code == 0,
               "#1758: fractional deleted object classification selection should keep inspection non-failing");
        expect_contains(deleted_object_process.stdout_text, "\"selectedReportObjectAvailable\": true",
                        "#1758: fractional deleted object classification should resolve selected objects");
        expect_contains_in_order(
            deleted_object_process.stdout_text,
            {
                "\"selectedReportObject\": {",
                "\"recordIndex\": 4",
                "\"deleted\": true"
            },
            "#1758: fractional deleted object classification should use integer portions");
        expect_contains(deleted_object_process.stdout_text, "\"objectTypeCode\": 5",
                        "#1758: fractional deleted object classification should preserve integer object type codes");
        expect_contains(deleted_object_process.stdout_text, "\"objectKind\": \"label\"",
                        "#1758: fractional deleted object classification should preserve integer object kinds");
        expect_contains(deleted_object_process.stdout_text, "\"objectCode\": 1",
                        "#1758: fractional deleted object classification should preserve integer object codes");
        expect_normalized_classification_preview_bounds(
            deleted_object_process.stdout_text,
            "#2312: selected fractional deleted object classification JSON");
    };

    run_fractional_classification_layout(temp_root / "fractional_classifications.frx",
                                         "fractional_classifications.frx",
                                         "report");
    run_fractional_classification_layout(temp_root / "fractional_classifications.lbx",
                                         "fractional_classifications.lbx",
                                         "label");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_trims_report_layout_classifications(
    const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_trimmed_report_layout_classification_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const auto run_trimmed_classification_layout = [&](const fs::path& asset_path,
                                                       const std::string& title,
                                                       const std::string& label) {
        write_synthetic_report_table_for_trimmed_classification_layout_json(asset_path);

        const auto summary_process = run_process_capture(
            studio_host_path,
            {"--path", asset_path.string(), "--json"},
            temp_root);

        if (summary_process.exit_code != 0) {
            std::cerr << "studio host " << label << " trimmed classification summary stdout:\n"
                      << summary_process.stdout_text << "\n";
            std::cerr << "studio host " << label << " trimmed classification summary stderr:\n"
                      << summary_process.stderr_text << "\n";
            std::cerr << "fixture root: " << temp_root << "\n";
        }

        expect(summary_process.exit_code == 0,
               "#1759: trimmed report/label layout classifications should keep inspection non-failing");
        expect_contains(summary_process.stdout_text, "\"documentTitle\": \"" + title + "\"",
                        "#1759: trimmed layout classifications should preserve document titles");
        if (asset_path.extension() == ".lbx") {
            expect_contains(summary_process.stdout_text, "\"isLabel\": true",
                            "#1759: trimmed classification label layouts should retain label identity");
        }
        expect_contains(summary_process.stdout_text, "\"pageSetupAvailable\": true",
                        "#1759: trimmed root OBJTYPE should expose settings summaries");
        expect_contains(summary_process.stdout_text, "\"orientationCode\": 0",
                        "#1759: trimmed root classifications should preserve settings values");
        expect_contains(summary_process.stdout_text, "\"paperSizeCode\": 1",
                        "#1759: trimmed root classifications should preserve later settings values");
        expect_contains(summary_process.stdout_text, "\"settingCount\": 2",
                        "#1759: trimmed root classifications should preserve live root settings");
        expect_normalized_classification_preview_bounds(
            summary_process.stdout_text,
            "#2312: trimmed classification summary JSON");
        expect_contains(summary_process.stdout_text, "\"sectionCount\": 1",
                        "#1759: trimmed band classifications should create live sections");
        expect_contains(summary_process.stdout_text, "\"deletedSectionCount\": 1",
                        "#1759: trimmed deleted band classifications should create deleted sections");
        expect_contains(summary_process.stdout_text, "\"liveObjectCount\": 1",
                        "#1759: trimmed object classifications should create live layout objects");
        expect_contains(summary_process.stdout_text, "\"deletedObjectCount\": 1",
                        "#1759: trimmed deleted object classifications should create deleted objects");
        expect_contains(summary_process.stdout_text, "{\"kind\": \"field\", \"count\": 1}",
                        "#1759: trimmed live object OBJTYPE should use trimmed values for kind counts");
        expect_contains(summary_process.stdout_text, "{\"kind\": \"label\", \"count\": 1}",
                        "#1759: trimmed deleted object OBJTYPE should use trimmed values for kind counts");
        expect_contains(summary_process.stdout_text, "{\"kind\": \"detail\", \"count\": 1}",
                        "#1759: trimmed live band OBJCODE should use trimmed values for kind counts");
        expect_contains(summary_process.stdout_text, "{\"kind\": \"page_footer\", \"count\": 1}",
                        "#1759: trimmed deleted band OBJCODE should use trimmed values for kind counts");

        const auto settings_process = run_process_capture(
            studio_host_path,
            {"--path", asset_path.string(), "--record", "0", "--json"},
            temp_root);
        expect(settings_process.exit_code == 0,
               "#1759: trimmed root classification selection should keep inspection non-failing");
        expect_contains(settings_process.stdout_text, "\"selectedReportSettingsAvailable\": true",
                        "#1759: trimmed root classification should resolve selected settings");
        expect_contains(settings_process.stdout_text, "\"selectedReportSelectionKind\": \"settings\"",
                        "#1759: trimmed root classification should expose settings selection kind");
        expect_normalized_classification_preview_bounds(
            settings_process.stdout_text,
            "#2359: selected trimmed settings classification JSON");

        const auto section_process = run_process_capture(
            studio_host_path,
            {"--path", asset_path.string(), "--record", "1", "--json"},
            temp_root);
        expect(section_process.exit_code == 0,
               "#1759: trimmed live section classification selection should keep inspection non-failing");
        expect_contains(section_process.stdout_text, "\"selectedReportSectionAvailable\": true",
                        "#1759: trimmed band classification should resolve selected sections");
        expect_contains(section_process.stdout_text, "\"selectedReportSelectionKind\": \"section\"",
                        "#1759: trimmed band classification should expose section selection kind");
        expect_contains(section_process.stdout_text, "\"bandKind\": \"detail\"",
                        "#1759: trimmed live section classification should preserve trimmed band kinds");
        expect_contains(section_process.stdout_text, "\"objectCode\": 4",
                        "#1759: trimmed live section classification should preserve trimmed band codes");
        expect_normalized_classification_preview_bounds(
            section_process.stdout_text,
            "#2312: selected trimmed live section classification JSON");

        const auto object_process = run_process_capture(
            studio_host_path,
            {"--path", asset_path.string(), "--record", "2", "--json"},
            temp_root);
        expect(object_process.exit_code == 0,
               "#1759: trimmed live object classification selection should keep inspection non-failing");
        expect_contains(object_process.stdout_text, "\"selectedReportObjectAvailable\": true",
                        "#1759: trimmed object classification should resolve selected objects");
        expect_contains(object_process.stdout_text, "\"selectedReportSelectionKind\": \"object\"",
                        "#1759: trimmed object classification should expose object selection kind");
        expect_contains(object_process.stdout_text, "\"objectTypeCode\": 8",
                        "#1759: trimmed live object classification should preserve trimmed object type codes");
        expect_contains(object_process.stdout_text, "\"objectKind\": \"field\"",
                        "#1759: trimmed live object classification should preserve trimmed object kinds");
        expect_contains(object_process.stdout_text, "\"objectCode\": 0",
                        "#1759: trimmed live object classification should preserve trimmed object codes");
        expect_contains(object_process.stdout_text, "\"containingSectionRecordIndex\": 1",
                        "#1759: trimmed live object classification should preserve containing sections");
        expect_normalized_classification_preview_bounds(
            object_process.stdout_text,
            "#2312: selected trimmed live object classification JSON");

        const auto deleted_section_process = run_process_capture(
            studio_host_path,
            {"--path", asset_path.string(), "--record", "3", "--json"},
            temp_root);
        expect(deleted_section_process.exit_code == 0,
               "#1759: trimmed deleted section classification selection should keep inspection non-failing");
        expect_contains(deleted_section_process.stdout_text, "\"selectedReportSectionAvailable\": true",
                        "#1759: trimmed deleted band classification should resolve selected sections");
        expect_contains(deleted_section_process.stdout_text, "\"bandKind\": \"page_footer\"",
                        "#1759: trimmed deleted section classification should preserve trimmed band kinds");
        expect_contains(deleted_section_process.stdout_text, "\"objectCode\": 7",
                        "#1759: trimmed deleted section classification should preserve trimmed band codes");
        expect_normalized_classification_preview_bounds(
            deleted_section_process.stdout_text,
            "#2312: selected trimmed deleted section classification JSON");

        const auto deleted_object_process = run_process_capture(
            studio_host_path,
            {"--path", asset_path.string(), "--record", "4", "--json"},
            temp_root);
        expect(deleted_object_process.exit_code == 0,
               "#1759: trimmed deleted object classification selection should keep inspection non-failing");
        expect_contains(deleted_object_process.stdout_text, "\"selectedReportObjectAvailable\": true",
                        "#1759: trimmed deleted object classification should resolve selected objects");
        expect_contains(deleted_object_process.stdout_text, "\"objectTypeCode\": 5",
                        "#1759: trimmed deleted object classification should preserve trimmed object type codes");
        expect_contains(deleted_object_process.stdout_text, "\"objectKind\": \"label\"",
                        "#1759: trimmed deleted object classification should preserve trimmed object kinds");
        expect_contains(deleted_object_process.stdout_text, "\"objectCode\": 1",
                        "#1759: trimmed deleted object classification should preserve trimmed object codes");
        expect_normalized_classification_preview_bounds(
            deleted_object_process.stdout_text,
            "#2312: selected trimmed deleted object classification JSON");
    };

    run_trimmed_classification_layout(temp_root / "trimmed_classifications.frx",
                                      "trimmed_classifications.frx",
                                      "report");
    run_trimmed_classification_layout(temp_root / "trimmed_classifications.lbx",
                                      "trimmed_classifications.lbx",
                                      "label");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_ignores_missing_report_layout_classification_fields(
    const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_missing_report_layout_classification_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const auto expect_no_report_selection = [](const std::string& stdout_text, const std::string& message_prefix) {
        expect_contains(stdout_text, "\"selectedReportSelectionAvailable\": false",
                        message_prefix + " should not expose report-selection availability");
        expect_contains(stdout_text, "\"selectedReportSelectionKind\": \"none\"",
                        message_prefix + " should expose explicit no-selection kind");
        expect_contains(stdout_text, "\"selectedReportObjectAvailable\": false",
                        message_prefix + " should not expose selected-object availability");
        expect_contains(stdout_text, "\"selectedReportObject\": null",
                        message_prefix + " should serialize null selected objects");
        expect_contains(stdout_text, "\"selectedReportObjectSectionAvailable\": false",
                        message_prefix + " should not expose containing-section availability");
        expect_contains(stdout_text, "\"selectedReportObjectSection\": null",
                        message_prefix + " should serialize null containing sections");
        expect_contains(stdout_text, "\"selectedReportSectionAvailable\": false",
                        message_prefix + " should not expose selected-section availability");
        expect_contains(stdout_text, "\"selectedReportSection\": null",
                        message_prefix + " should serialize null selected sections");
        expect_contains(stdout_text, "\"selectedReportSettingsAvailable\": false",
                        message_prefix + " should not expose selected-settings availability");
        expect_contains(stdout_text, "\"selectedReportSettings\": null",
                        message_prefix + " should serialize null selected settings");
        expect_empty_report_layout_preview_bounds(
            stdout_text,
            "#2314: " + message_prefix + " selected missing classification schema JSON");
    };

    const auto run_missing_classification_layout = [&](const fs::path& asset_path,
                                                       const std::string& title,
                                                       const std::string& label) {
        write_synthetic_report_table_for_missing_classification_layout_json(asset_path);

        const auto summary_process = run_process_capture(
            studio_host_path,
            {"--path", asset_path.string(), "--json"},
            temp_root);

        if (summary_process.exit_code != 0) {
            std::cerr << "studio host " << label << " missing classification summary stdout:\n"
                      << summary_process.stdout_text << "\n";
            std::cerr << "studio host " << label << " missing classification summary stderr:\n"
                      << summary_process.stderr_text << "\n";
            std::cerr << "fixture root: " << temp_root << "\n";
        }

        expect(summary_process.exit_code == 0,
               "#1721: missing report/label layout classification fields should keep inspection non-failing");
        expect_contains(summary_process.stdout_text, "\"documentTitle\": \"" + title + "\"",
                        "#1721: missing classification layouts should preserve document titles");
        if (asset_path.extension() == ".lbx") {
            expect_contains(summary_process.stdout_text, "\"isLabel\": true",
                            "#1721: missing classification label layouts should retain label identity");
        }
        expect_contains(summary_process.stdout_text, "\"previewBoundsAvailable\": false",
                        "#1721: missing classification rows should not create live preview bounds");
        expect_contains(summary_process.stdout_text, "\"deletedPreviewBoundsAvailable\": false",
                        "#1721: missing classification rows should not create deleted preview bounds");
        expect_empty_report_layout_preview_bounds(
            summary_process.stdout_text,
            "#2344: missing classification summary JSON");
        expect_contains(summary_process.stdout_text, "\"pageSetupAvailable\": false",
                        "#1721: missing classification rows should not infer root settings");
        expect_contains(summary_process.stdout_text, "\"liveObjectCount\": 0",
                        "#1721: missing classification rows should not create live layout objects");
        expect_contains(summary_process.stdout_text, "\"deletedObjectCount\": 0",
                        "#1721: missing classification rows should not create deleted layout objects");
        expect_contains(summary_process.stdout_text, "\"sectionCount\": 0",
                        "#1721: missing classification rows should not create live sections");
        expect_contains(summary_process.stdout_text, "\"deletedSectionCount\": 0",
                        "#1721: missing classification rows should not create deleted sections");
        expect_contains(summary_process.stdout_text, "\"settingCount\": 0",
                        "#1721: missing classification rows should not create live root settings");
        expect_contains(summary_process.stdout_text, "\"deletedSettingCount\": 0",
                        "#1721: missing classification rows should not create deleted root settings");

        for (const auto record_index : {0, 1, 2, 3}) {
            const auto selected_process = run_process_capture(
                studio_host_path,
                {"--path", asset_path.string(), "--record", std::to_string(record_index), "--json"},
                temp_root);

            expect(selected_process.exit_code == 0,
                   "#1721: missing classification record selection should keep inspection non-failing");
            expect_no_report_selection(
                selected_process.stdout_text,
                "#1721: missing classification record " + std::to_string(record_index));
        }
    };

    run_missing_classification_layout(temp_root / "missing_classification.frx",
                                      "missing_classification.frx",
                                      "report");
    run_missing_classification_layout(temp_root / "missing_classification.lbx",
                                      "missing_classification.lbx",
                                      "label");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_ignores_missing_report_layout_objtype_schema(
    const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_missing_report_layout_objtype_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const auto expect_no_report_selection = [](const std::string& stdout_text, const std::string& message_prefix) {
        expect_contains(stdout_text, "\"selectedReportSelectionAvailable\": false",
                        message_prefix + " should not expose report-selection availability");
        expect_contains(stdout_text, "\"selectedReportSelectionKind\": \"none\"",
                        message_prefix + " should expose explicit no-selection kind");
        expect_contains(stdout_text, "\"selectedReportObjectAvailable\": false",
                        message_prefix + " should not expose selected-object availability");
        expect_contains(stdout_text, "\"selectedReportObject\": null",
                        message_prefix + " should serialize null selected objects");
        expect_contains(stdout_text, "\"selectedReportObjectSectionAvailable\": false",
                        message_prefix + " should not expose containing-section availability");
        expect_contains(stdout_text, "\"selectedReportObjectSection\": null",
                        message_prefix + " should serialize null containing sections");
        expect_contains(stdout_text, "\"selectedReportSectionAvailable\": false",
                        message_prefix + " should not expose selected-section availability");
        expect_contains(stdout_text, "\"selectedReportSection\": null",
                        message_prefix + " should serialize null selected sections");
        expect_contains(stdout_text, "\"selectedReportSettingsAvailable\": false",
                        message_prefix + " should not expose selected-settings availability");
        expect_contains(stdout_text, "\"selectedReportSettings\": null",
                        message_prefix + " should serialize null selected settings");
        expect_empty_report_layout_preview_bounds(
            stdout_text,
            "#2314: " + message_prefix + " selected OBJCODE-only schema JSON");
    };

    const auto run_missing_objtype_layout = [&](const fs::path& asset_path,
                                                const std::string& title,
                                                const std::string& label) {
        write_synthetic_report_table_for_missing_objtype_layout_json(asset_path);

        const auto summary_process = run_process_capture(
            studio_host_path,
            {"--path", asset_path.string(), "--json"},
            temp_root);

        if (summary_process.exit_code != 0) {
            std::cerr << "studio host " << label << " missing OBJTYPE summary stdout:\n"
                      << summary_process.stdout_text << "\n";
            std::cerr << "studio host " << label << " missing OBJTYPE summary stderr:\n"
                      << summary_process.stderr_text << "\n";
            std::cerr << "fixture root: " << temp_root << "\n";
        }

        expect(summary_process.exit_code == 0,
               "#1731: missing OBJTYPE schema should keep report/label inspection non-failing");
        expect_contains(summary_process.stdout_text, "\"documentTitle\": \"" + title + "\"",
                        "#1731: missing OBJTYPE layouts should preserve document titles");
        if (asset_path.extension() == ".lbx") {
            expect_contains(summary_process.stdout_text, "\"isLabel\": true",
                            "#1731: missing OBJTYPE label layouts should retain label identity");
        }
        expect_contains(summary_process.stdout_text, "\"previewBoundsAvailable\": false",
                        "#1731: OBJCODE-only rows should not create live preview bounds");
        expect_contains(summary_process.stdout_text, "\"deletedPreviewBoundsAvailable\": false",
                        "#1731: OBJCODE-only rows should not create deleted preview bounds");
        expect_empty_report_layout_preview_bounds(
            summary_process.stdout_text,
            "#2344: OBJCODE-only summary JSON");
        expect_contains(summary_process.stdout_text, "\"pageSetupAvailable\": false",
                        "#1731: OBJCODE-only rows should not infer root settings");
        expect_contains(summary_process.stdout_text, "\"liveObjectCount\": 0",
                        "#1731: OBJCODE-only rows should not create live layout objects");
        expect_contains(summary_process.stdout_text, "\"deletedObjectCount\": 0",
                        "#1731: OBJCODE-only rows should not create deleted layout objects");
        expect_contains(summary_process.stdout_text, "\"sectionCount\": 0",
                        "#1731: OBJCODE-only rows should not create live sections");
        expect_contains(summary_process.stdout_text, "\"deletedSectionCount\": 0",
                        "#1731: OBJCODE-only rows should not create deleted sections");
        expect_contains(summary_process.stdout_text, "\"settingCount\": 0",
                        "#1731: OBJCODE-only rows should not create live root settings");
        expect_contains(summary_process.stdout_text, "\"deletedSettingCount\": 0",
                        "#1731: OBJCODE-only rows should not create deleted root settings");

        for (const auto record_index : {0, 1, 2, 3}) {
            const auto selected_process = run_process_capture(
                studio_host_path,
                {"--path", asset_path.string(), "--record", std::to_string(record_index), "--json"},
                temp_root);

            expect(selected_process.exit_code == 0,
                   "#1731: OBJCODE-only record selection should keep inspection non-failing");
            expect_no_report_selection(
                selected_process.stdout_text,
                "#1731: OBJCODE-only record " + std::to_string(record_index));
        }
    };

    run_missing_objtype_layout(temp_root / "missing_objtype.frx",
                               "missing_objtype.frx",
                               "report");
    run_missing_objtype_layout(temp_root / "missing_objtype.lbx",
                               "missing_objtype.lbx",
                               "label");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_exposes_unknown_report_band_codes(const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_unknown_report_band_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    ScopedDefaultLocaleCatalogEnvironment default_locale_environment;

    const auto run_unknown_band_layout = [&](const fs::path& asset_path,
                                             const std::string& title,
                                             const std::string& label) {
        write_synthetic_report_table_for_unknown_band_layout_json(asset_path);

        const auto summary_process = run_process_capture(
            studio_host_path,
            {"--path", asset_path.string(), "--json"},
            temp_root);

        if (summary_process.exit_code != 0) {
            std::cerr << "studio host " << label << " unknown band summary stdout:\n"
                      << summary_process.stdout_text << "\n";
            std::cerr << "studio host " << label << " unknown band summary stderr:\n"
                      << summary_process.stderr_text << "\n";
            std::cerr << "fixture root: " << temp_root << "\n";
        }

        expect(summary_process.exit_code == 0,
               "#1722: unknown report/label band codes should keep inspection non-failing");
        expect_contains(summary_process.stdout_text, "\"documentTitle\": \"" + title + "\"",
                        "#1722: unknown band layouts should preserve document titles");
        if (asset_path.extension() == ".lbx") {
            expect_contains(summary_process.stdout_text, "\"isLabel\": true",
                            "#1722: unknown band label layouts should retain label identity");
        }
        expect_contains(summary_process.stdout_text, "\"previewBoundsAvailable\": true",
                        "#1722: unknown live bands should still contribute preview bounds");
        expect_contains(summary_process.stdout_text, "\"previewBoundsTop\": 300",
                        "#1722: unknown live band top bounds should be preserved");
        expect_contains(summary_process.stdout_text, "\"previewBoundsBottom\": 1000",
                        "#1722: unknown live band bottom bounds should be preserved");
        expect_contains(summary_process.stdout_text, "\"deletedPreviewBoundsAvailable\": true",
                        "#1722: unknown deleted bands should still contribute deleted preview bounds");
        expect_contains(summary_process.stdout_text, "\"deletedPreviewBoundsTop\": 1200",
                        "#1722: unknown deleted band top bounds should be preserved");
        expect_contains(summary_process.stdout_text, "\"deletedPreviewBoundsBottom\": 1600",
                        "#1722: unknown deleted band bottom bounds should be preserved");
        expect_unknown_band_preview_bounds(
            summary_process.stdout_text,
            "#2346: unknown band summary JSON");
        expect_contains(summary_process.stdout_text, "\"sectionCount\": 1",
                        "#1722: unknown live bands should remain section rows");
        expect_contains(summary_process.stdout_text, "\"deletedSectionCount\": 1",
                        "#1722: unknown deleted bands should remain deleted-section rows");
        expect_contains(summary_process.stdout_text, "\"sectionKindCounts\": [\n        {\"kind\": \"other\", \"count\": 1}\n      ]",
                        "#1722: unknown live bands should summarize under the other kind");
        expect_contains(summary_process.stdout_text, "\"deletedSectionKindCounts\": [\n        {\"kind\": \"other\", \"count\": 1}\n      ]",
                        "#1722: unknown deleted bands should summarize under the other kind");
        expect_contains(summary_process.stdout_text, "\"sectionHeightTotal\": 700",
                        "#1722: unknown live band heights should be preserved");
        expect_contains(summary_process.stdout_text, "\"deletedSectionHeightTotal\": 400",
                        "#1722: unknown deleted band heights should be preserved");

        const auto live_process = run_process_capture(
            studio_host_path,
            {"--path", asset_path.string(), "--record", "1", "--json"},
            temp_root);

        expect(live_process.exit_code == 0,
               "#1722: unknown live band record selection should keep inspection non-failing");
        expect_contains(live_process.stdout_text, "\"selectedReportSectionAvailable\": true",
                        "#1722: unknown live band selections should expose selected sections");
        expect_unknown_band_preview_bounds(
            live_process.stdout_text,
            "#2346: selected unknown live band JSON");
        expect_contains_in_order(
            live_process.stdout_text,
            {
                "\"selectedReportSection\": {",
                "\"id\": \"unknown-live-band-guid\"",
                "\"title\": \"Other Band\"",
                "\"bandKind\": \"other\"",
                "\"expression\": \"unknown.live\"",
                "\"recordIndex\": 1",
                "\"deleted\": false",
                "\"objectCode\": 999",
                "\"top\": 300",
                "\"height\": 700",
                "\"bottom\": 1000"
            },
            "#1722: unknown live band selections should serialize explicit other-band metadata");

        const auto deleted_process = run_process_capture(
            studio_host_path,
            {"--path", asset_path.string(), "--record", "2", "--json"},
            temp_root);

        expect(deleted_process.exit_code == 0,
               "#1722: unknown deleted band record selection should keep inspection non-failing");
        expect_contains(deleted_process.stdout_text, "\"selectedReportSectionAvailable\": true",
                        "#1722: unknown deleted band selections should expose selected sections");
        expect_unknown_band_preview_bounds(
            deleted_process.stdout_text,
            "#2346: selected unknown deleted band JSON");
        expect_contains_in_order(
            deleted_process.stdout_text,
            {
                "\"deletedSections\": [",
                "\"id\": \"unknown-deleted-band-guid\"",
                "\"title\": \"Other Band\"",
                "\"bandKind\": \"other\"",
                "\"expression\": \"unknown.deleted\"",
                "\"selectedReportSection\": {",
                "\"id\": \"unknown-deleted-band-guid\"",
                "\"title\": \"Other Band\"",
                "\"bandKind\": \"other\"",
                "\"expression\": \"unknown.deleted\"",
                "\"recordIndex\": 2",
                "\"deleted\": true",
                "\"objectCode\": 1234",
                "\"top\": 1200",
                "\"height\": 400",
                "\"bottom\": 1600"
            },
            "#1722: unknown deleted band selections should serialize explicit other-band metadata");
    };

    run_unknown_band_layout(temp_root / "unknown_band.frx",
                            "unknown_band.frx",
                            "report");
    run_unknown_band_layout(temp_root / "unknown_band.lbx",
                            "unknown_band.lbx",
                            "label");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_ignores_invalid_direct_report_page_setup_fields(
    const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_invalid_direct_page_setup_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const auto run_invalid_direct_page_setup_layout = [&](const fs::path& asset_path,
                                                          const std::string& title,
                                                          const std::string& label) {
        write_synthetic_report_table_for_invalid_direct_page_setup_layout_json(asset_path);

        const auto summary_process = run_process_capture(
            studio_host_path,
            {"--path", asset_path.string(), "--json"},
            temp_root);

        if (summary_process.exit_code != 0) {
            std::cerr << "studio host " << label << " invalid direct page setup summary stdout:\n"
                      << summary_process.stdout_text << "\n";
            std::cerr << "studio host " << label << " invalid direct page setup summary stderr:\n"
                      << summary_process.stderr_text << "\n";
            std::cerr << "fixture root: " << temp_root << "\n";
        }

        expect(summary_process.exit_code == 0,
               "#1733: invalid direct page setup fields should keep report/label inspection non-failing");
        expect_contains(summary_process.stdout_text, "\"documentTitle\": \"" + title + "\"",
                        "#1733: invalid direct page setup layouts should preserve document titles");
        if (asset_path.extension() == ".lbx") {
            expect_contains(summary_process.stdout_text, "\"isLabel\": true",
                            "#1733: invalid direct page setup label layouts should retain label identity");
        }
        expect_contains(summary_process.stdout_text, "\"pageSetupAvailable\": false",
                        "#1733: invalid direct page setup fields should not fabricate page setup availability");
        expect_contains(summary_process.stdout_text, "\"orientationAvailable\": false",
                        "#1733: invalid direct orientation should not advertise orientation availability");
        expect_contains(summary_process.stdout_text, "\"paperSizeAvailable\": false",
                        "#1733: invalid direct paper size should not advertise paper-size availability");
        expect_contains(summary_process.stdout_text, "\"topMarginAvailable\": false",
                        "#1733: invalid direct top margin should not advertise top-margin availability");
        expect_contains(summary_process.stdout_text, "\"settingCount\": 3",
                        "#1733: invalid direct settings should still be counted as live raw settings");
        expect_contains(summary_process.stdout_text, "\"deletedSettingCount\": 3",
                        "#1733: invalid direct settings should still be counted as deleted raw settings");
        expect_contains(summary_process.stdout_text,
                        "\"name\": \"ORIENTATION\", \"recordIndex\": 0, \"fieldIndex\": 2, \"sourceLineIndex\": null, \"memoBlockNumber\": 0, \"value\": \"2.bad\"",
                        "#1733: invalid direct orientation provenance should remain inspectable");
        expect_contains(summary_process.stdout_text,
                        "\"name\": \"PAPERSIZE\", \"recordIndex\": 0, \"fieldIndex\": 3, \"sourceLineIndex\": null, \"memoBlockNumber\": 0, \"value\": \"9.1junk\"",
                        "#1733: invalid direct paper-size provenance should remain inspectable");
        expect_contains(summary_process.stdout_text,
                        "\"name\": \"TOPMARGIN\", \"recordIndex\": 0, \"fieldIndex\": 4, \"sourceLineIndex\": null, \"memoBlockNumber\": 0, \"value\": \"100.2.3\"",
                        "#1733: invalid direct top-margin provenance should remain inspectable");
        expect_contains(summary_process.stdout_text, "\"orientationCode\": 0",
                        "#1733: invalid direct orientation should keep the default orientation code inert");
        expect_contains(summary_process.stdout_text, "\"paperSizeCode\": 0",
                        "#1733: invalid direct paper size should keep the default paper-size code inert");
        expect_contains(summary_process.stdout_text, "\"topMargin\": 0",
                        "#1733: invalid direct top margin should keep the default top-margin value inert");
        expect_empty_report_layout_preview_bounds(
            summary_process.stdout_text,
            "#2318: invalid direct page setup summary JSON");

        const auto live_process = run_process_capture(
            studio_host_path,
            {"--path", asset_path.string(), "--record", "0", "--json"},
            temp_root);

        expect(live_process.exit_code == 0,
               "#1733: invalid direct live settings selection should keep inspection non-failing");
        expect_contains(live_process.stdout_text, "\"selectedReportSettingsAvailable\": true",
                        "#1733: invalid direct live settings should advertise selected-settings availability");
        expect_contains(live_process.stdout_text, "\"selectedReportSelectionKind\": \"settings\"",
                        "#1733: invalid direct live settings should expose settings selection kind");
        expect_empty_report_layout_preview_bounds(
            live_process.stdout_text,
            "#2318: selected invalid direct live settings JSON");
        expect_contains_in_order(
            live_process.stdout_text,
            {
                "\"selectedReportSettings\": [",
                "\"name\": \"ORIENTATION\"",
                "\"recordIndex\": 0",
                "\"fieldIndex\": 2",
                "\"value\": \"2.bad\"",
                "\"name\": \"PAPERSIZE\"",
                "\"recordIndex\": 0",
                "\"fieldIndex\": 3",
                "\"value\": \"9.1junk\"",
                "\"name\": \"TOPMARGIN\"",
                "\"recordIndex\": 0",
                "\"fieldIndex\": 4",
                "\"value\": \"100.2.3\""
            },
            "#1733: invalid direct live selection should expose raw selected-settings metadata");

        const auto deleted_process = run_process_capture(
            studio_host_path,
            {"--path", asset_path.string(), "--record", "1", "--json"},
            temp_root);

        expect(deleted_process.exit_code == 0,
               "#1733: invalid direct deleted settings selection should keep inspection non-failing");
        expect_contains(deleted_process.stdout_text, "\"selectedReportSettingsAvailable\": true",
                        "#1733: invalid direct deleted settings should advertise selected-settings availability");
        expect_contains(deleted_process.stdout_text, "\"selectedReportSelectionKind\": \"settings\"",
                        "#1733: invalid direct deleted settings should expose settings selection kind");
        expect_empty_report_layout_preview_bounds(
            deleted_process.stdout_text,
            "#2318: selected invalid direct deleted settings JSON");
        expect_contains_in_order(
            deleted_process.stdout_text,
            {
                "\"selectedReportSettings\": [",
                "\"name\": \"ORIENTATION\"",
                "\"recordIndex\": 1",
                "\"fieldIndex\": 2",
                "\"value\": \"3.bad\"",
                "\"name\": \"PAPERSIZE\"",
                "\"recordIndex\": 1",
                "\"fieldIndex\": 3",
                "\"value\": \"10.1junk\"",
                "\"name\": \"TOPMARGIN\"",
                "\"recordIndex\": 1",
                "\"fieldIndex\": 4",
                "\"value\": \"200.2.3\""
            },
            "#1733: invalid direct deleted selection should expose raw selected-settings metadata");
    };

    run_invalid_direct_page_setup_layout(temp_root / "invalid_direct_page_setup.frx",
                                         "invalid_direct_page_setup.frx",
                                         "report");
    run_invalid_direct_page_setup_layout(temp_root / "invalid_direct_page_setup.lbx",
                                         "invalid_direct_page_setup.lbx",
                                         "label");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}
#endif

}  // namespace cf_test_studio_host_json
