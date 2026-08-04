// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.

#include "copperfin/vfp/dbf_table.h"
#include "copperfin/vfp/visual_asset_editor.h"
#include "test_locale_catalog_environment_support.h"
#include "test_process_capture_support.h"

#include <cstdint>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

namespace {

int failures = 0;
using copperfin::test_support::ScopedDefaultLocaleCatalogEnvironment;

void expect(bool condition, const std::string& message) {
    if (!condition) {
        std::cerr << "FAIL: " << message << "\n";
        ++failures;
    }
}

void expect_contains(const std::string& text, const std::string& needle, const std::string& message) {
    expect(text.find(needle) != std::string::npos, message);
}

void expect_not_contains(const std::string& text, const std::string& needle, const std::string& message) {
    expect(text.find(needle) == std::string::npos, message);
}

void expect_contains_in_order(
    const std::string& text,
    const std::vector<std::string>& needles,
    const std::string& message) {
    std::size_t offset = 0U;
    for (const auto& needle : needles) {
        const std::size_t position = text.find(needle, offset);
        if (position == std::string::npos) {
            expect(false, message);
            return;
        }
        offset = position + needle.size();
    }
}

void expect_full_report_layout_preview_bounds(const std::string& text, const std::string& prefix) {
    expect_contains(text, "\"previewBoundsAvailable\": true",
                    prefix + " should preserve live preview availability");
    expect_contains(text, "\"previewBoundsLeft\": 0",
                    prefix + " should preserve live preview left bounds");
    expect_contains(text, "\"previewBoundsTop\": 0",
                    prefix + " should preserve live preview top bounds");
    expect_contains(text, "\"previewBoundsRight\": 5200",
                    prefix + " should preserve live preview right bounds");
    expect_contains(text, "\"previewBoundsBottom\": 8100",
                    prefix + " should preserve live preview bottom bounds");
    expect_contains(text, "\"previewBoundsWidth\": 5200",
                    prefix + " should preserve live preview widths");
    expect_contains(text, "\"previewBoundsHeight\": 8100",
                    prefix + " should preserve live preview heights");
    expect_contains(text, "\"deletedPreviewBoundsAvailable\": true",
                    prefix + " should preserve deleted preview availability");
    expect_contains(text, "\"deletedPreviewBoundsLeft\": 1000",
                    prefix + " should preserve deleted preview left bounds");
    expect_contains(text, "\"deletedPreviewBoundsTop\": 2600",
                    prefix + " should preserve deleted preview top bounds");
    expect_contains(text, "\"deletedPreviewBoundsRight\": 2200",
                    prefix + " should preserve deleted preview right bounds");
    expect_contains(text, "\"deletedPreviewBoundsBottom\": 2900",
                    prefix + " should preserve deleted preview bottom bounds");
    expect_contains(text, "\"deletedPreviewBoundsWidth\": 1200",
                    prefix + " should preserve deleted preview widths");
    expect_contains(text, "\"deletedPreviewBoundsHeight\": 300",
                    prefix + " should preserve deleted preview heights");
}


void expect_updated_settings_page_summary(const std::string& text, const std::string& prefix) {
    expect_contains(text, "\"pageSetupAvailable\": true",
                    prefix + " should expose effective page setup availability");
    expect_contains(text, "\"orientationAvailable\": true",
                    prefix + " should expose orientation availability");
    expect_contains(text, "\"orientationCode\": 1",
                    prefix + " should expose the orientation code");
    expect_contains(text, "\"paperSizeAvailable\": true",
                    prefix + " should expose paper-size availability");
    expect_contains(text, "\"paperSizeCode\": 9",
                    prefix + " should expose the paper-size code");
    expect_contains(text, "\"topMarginAvailable\": true",
                    prefix + " should expose top-margin availability");
    expect_contains(text, "\"topMargin\": 14",
                    prefix + " should expose the top margin");
    expect_contains(text, "\"bottomMarginAvailable\": true",
                    prefix + " should expose bottom-margin availability");
    expect_contains(text, "\"bottomMargin\": 32",
                    prefix + " should expose the bottom margin");
    expect_contains(text, "\"gridVerticalAvailable\": true",
                    prefix + " should expose vertical-grid availability");
    expect_contains(text, "\"gridVertical\": 6",
                    prefix + " should expose vertical grid spacing");
    expect_contains(text, "\"gridHorizontalAvailable\": true",
                    prefix + " should expose horizontal-grid availability");
    expect_contains(text, "\"gridHorizontal\": 10",
                    prefix + " should expose horizontal grid spacing");
}

void expect_cleared_settings_page_summary(const std::string& text, const std::string& prefix) {
    expect_contains(text, "\"pageSetupAvailable\": true",
                    prefix + " should preserve effective direct-field page setup");
    expect_contains(text, "\"orientationAvailable\": false",
                    prefix + " should clear orientation availability");
    expect_contains(text, "\"orientationCode\": 0",
                    prefix + " should reset the orientation code");
    expect_contains(text, "\"paperSizeAvailable\": false",
                    prefix + " should clear paper-size availability");
    expect_contains(text, "\"paperSizeCode\": 0",
                    prefix + " should reset the paper-size code");
    expect_contains(text, "\"topMarginAvailable\": true",
                    prefix + " should preserve direct top-margin availability");
    expect_contains(text, "\"topMargin\": 10",
                    prefix + " should preserve the direct top margin");
    expect_contains(text, "\"bottomMarginAvailable\": false",
                    prefix + " should clear bottom-margin availability");
    expect_contains(text, "\"bottomMargin\": 0",
                    prefix + " should reset the bottom margin");
    expect_contains(text, "\"gridVerticalAvailable\": false",
                    prefix + " should clear vertical-grid availability");
    expect_contains(text, "\"gridVertical\": 0",
                    prefix + " should reset vertical grid spacing");
    expect_contains(text, "\"gridHorizontalAvailable\": false",
                    prefix + " should clear horizontal-grid availability");
    expect_contains(text, "\"gridHorizontal\": 0",
                    prefix + " should reset horizontal grid spacing");
}

std::string normalize_line_endings(std::string text) {
    std::string normalized;
    normalized.reserve(text.size());
    for (std::size_t index = 0; index < text.size(); ++index) {
        if (text[index] == '\r') {
            if (index + 1U < text.size() && text[index + 1U] == '\n') {
                continue;
            }
            normalized.push_back('\n');
        } else {
            normalized.push_back(text[index]);
        }
    }
    return normalized;
}

using ProcessResult = copperfin::test_support::CapturedProcessResult;

ProcessResult run_process_capture(
    const std::string& executable_path,
    const std::vector<std::string>& arguments,
    const std::filesystem::path& working_directory) {
    return copperfin::test_support::normalize_captured_process_line_endings(
        copperfin::test_support::run_process_capture(
            copperfin::test_support::path_from_utf8_string(executable_path),
            arguments,
            working_directory));
}

void write_synthetic_report_table_for_layout_json(const std::filesystem::path& report_path) {
    const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
        {.name = "OBJTYPE", .type = 'N', .length = 8U},
        {.name = "OBJCODE", .type = 'N', .length = 8U},
        {.name = "EXPR", .type = 'M', .length = 4U},
        {.name = "HPOS", .type = 'N', .length = 10U},
        {.name = "VPOS", .type = 'N', .length = 10U},
        {.name = "WIDTH", .type = 'N', .length = 10U},
        {.name = "HEIGHT", .type = 'N', .length = 10U},
        {.name = "FONTFACE", .type = 'M', .length = 4U},
        {.name = "TOPMARGIN", .type = 'N', .length = 10U},
        {.name = "UNIQUEID", .type = 'C', .length = 24U}
    };
    const std::vector<std::vector<std::string>> records{
        {"1", "53", "ORIENTATION=0\nPAPERSIZE=1\nBOTMARGIN=20\nGRIDV=4\nGRIDH=8", "", "", "", "", "", "10", ""},
        {"9", "1", "", "", "0", "", "2000", "", "", ""},
        {"9", "4", "", "", "2000", "", "5000", "", "", ""},
        {"8", "0", "customer.company", "1200", "2600", "4000", "450", "Segoe UI", "", "field-guid"},
        {"5", "", "\"Invoice\"", "900", "100", "1800", "350", "", "", "label-guid"},
        {"6", "", "", "50", "8000", "100", "100", "", "", ""},
        {"5", "", "\"Deleted label\"", "1000", "2600", "1200", "300", "", "", ""}
    };

    const auto create_result = copperfin::vfp::create_dbf_table_file(report_path.string(), fields, records);
    expect(create_result.ok, "#2784: synthetic report/label layout settings fixture should be created");

    const auto delete_result = copperfin::vfp::set_record_deleted_flag(report_path.string(), 6U, true);
    expect(delete_result.ok, "#2784: synthetic report/label layout settings fixture should mark deleted objects");
}

void write_synthetic_report_table_for_deleted_settings_json(const std::filesystem::path& report_path) {
    write_synthetic_report_table_for_layout_json(report_path);
    const auto delete_result = copperfin::vfp::set_record_deleted_flag(report_path.string(), 0U, true);
    expect(delete_result.ok, "#2784: deleted settings fixture should mark report settings deleted");
}

void write_synthetic_report_table_for_unsupported_settings_json(const std::filesystem::path& report_path) {
    const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
        {.name = "OBJTYPE", .type = 'N', .length = 8U},
        {.name = "OBJCODE", .type = 'N', .length = 8U},
        {.name = "EXPR", .type = 'M', .length = 4U},
        {.name = "HPOS", .type = 'N', .length = 10U},
        {.name = "VPOS", .type = 'N', .length = 10U},
        {.name = "WIDTH", .type = 'N', .length = 10U},
        {.name = "HEIGHT", .type = 'N', .length = 10U},
        {.name = "FONTFACE", .type = 'M', .length = 4U},
        {.name = "TOPMARGIN", .type = 'N', .length = 10U},
        {.name = "UNIQUEID", .type = 'C', .length = 24U}
    };
    const std::vector<std::vector<std::string>> records{
        {"1", "53", "ORIENTATION=0\n* keep-this-comment\n\nPAPERSIZE=1\nXUSER=keepme\nBOTMARGIN=20\nGRIDV=4\nGRIDH=8", "", "", "", "", "", "10", ""},
        {"9", "1", "", "", "0", "", "2000", "", "", ""},
        {"9", "4", "", "", "2000", "", "5000", "", "", ""},
        {"8", "0", "customer.company", "1200", "2600", "4000", "450", "Segoe UI", "", "field-guid"},
        {"5", "", "\"Invoice\"", "900", "100", "1800", "350", "", "", "label-guid"},
        {"6", "", "", "50", "8000", "100", "100", "", "", ""},
        {"5", "", "\"Deleted label\"", "1000", "2600", "1200", "300", "", "", ""}
    };

    const auto create_result = copperfin::vfp::create_dbf_table_file(report_path.string(), fields, records);
    expect(create_result.ok, "#3097: unsupported settings memo fixture should be created");
    const auto delete_result = copperfin::vfp::set_record_deleted_flag(report_path.string(), 6U, true);
    expect(delete_result.ok, "#3097: unsupported settings memo fixture should mark deleted objects");
}

void write_synthetic_report_table_for_deleted_unsupported_settings_json(const std::filesystem::path& report_path) {
    write_synthetic_report_table_for_unsupported_settings_json(report_path);
    const auto delete_result = copperfin::vfp::set_record_deleted_flag(report_path.string(), 0U, true);
    expect(delete_result.ok, "#3097: deleted unsupported settings fixture should mark report settings deleted");
}

void run_settings_memo_update_case(
    const std::string& studio_host_path,
    const std::filesystem::path& temp_root,
    const std::string& asset_stem,
    const std::string& extension,
    const std::string& updated_settings,
    bool deleted,
    const std::string& label,
    const std::string& issue_prefix) {
    const std::filesystem::path asset_path = temp_root / (asset_stem + extension);
    if (deleted) {
        write_synthetic_report_table_for_deleted_settings_json(asset_path);
    } else {
        write_synthetic_report_table_for_layout_json(asset_path);
    }

    const auto update_process = run_process_capture(
        studio_host_path,
        {
            "--path", copperfin::test_support::path_to_utf8_string(asset_path),
            "--set-property",
            "--record", "0",
            "--property-name", "EXPR",
            "--property-value", updated_settings,
            "--json"
        },
        temp_root);

    if (update_process.exit_code != 0) {
        std::cerr << "studio host " << label << " settings memo update " << extension << " stdout:\n"
                  << update_process.stdout_text << "\n";
        std::cerr << "studio host " << label << " settings memo update " << extension << " stderr:\n"
                  << update_process.stderr_text << "\n";
        std::cerr << "fixture root: " << temp_root << "\n";
    }

    expect(update_process.exit_code == 0, issue_prefix + " update should exit successfully");
    const auto expr_property = copperfin::vfp::query_visual_object_property({
        .path = asset_path.string(),
        .record_index = 0U,
        .object_name = {},
        .unique_id = {},
        .property_name = "EXPR"
    });
    expect(expr_property.ok && expr_property.exists && expr_property.value == updated_settings,
           issue_prefix + " update should persist the EXPR memo field");
    expect_contains(update_process.stdout_text, "\"documentTitle\": \"" + asset_path.filename().string() + "\"",
                    issue_prefix + " update should return refreshed report-layout JSON");
    if (asset_path.extension() == ".lbx") {
        expect_contains(update_process.stdout_text, "\"isLabel\": true",
                        issue_prefix + " update should retain label identity");
    }
    expect_full_report_layout_preview_bounds(
        update_process.stdout_text,
        deleted ? "#2038: record-selected deleted report/label settings memo update JSON"
                : "#2038: record-selected report/label settings memo update JSON");
    expect_updated_settings_page_summary(update_process.stdout_text, issue_prefix + " update");
    if (!deleted) {
        expect_contains(update_process.stdout_text, "\"settingCount\": 7",
                        issue_prefix + " update should preserve memo and field setting counts");
        expect_contains_in_order(
            update_process.stdout_text,
            {
                "\"settings\": [",
                "\"name\": \"ORIENTATION\", \"recordIndex\": 0, \"fieldIndex\": 2, \"sourceLineIndex\": 0",
                "\"name\": \"PAPERSIZE\", \"recordIndex\": 0, \"fieldIndex\": 2, \"sourceLineIndex\": 1",
                "\"name\": \"BOTMARGIN\", \"recordIndex\": 0, \"fieldIndex\": 2, \"sourceLineIndex\": 2",
                "\"name\": \"GRIDV\", \"recordIndex\": 0, \"fieldIndex\": 2, \"sourceLineIndex\": 3",
                "\"name\": \"GRIDH\", \"recordIndex\": 0, \"fieldIndex\": 2, \"sourceLineIndex\": 4",
                "\"name\": \"TOPMARGIN\", \"recordIndex\": 0, \"fieldIndex\": 2, \"sourceLineIndex\": 5",
                "\"name\": \"TOPMARGIN\", \"recordIndex\": 0, \"fieldIndex\": 8, \"sourceLineIndex\": null"
            },
            issue_prefix + " update should refresh memo and field setting provenance");
    } else {
        expect_contains(update_process.stdout_text, "\"settingCount\": 0",
                        issue_prefix + " update should not fabricate live settings");
        expect_contains(update_process.stdout_text, "\"deletedSettingCount\": 7",
                        issue_prefix + " update should refresh deleted setting counts");
        expect_contains(update_process.stdout_text, "\"selectedReportSettingsAvailable\": true",
                        issue_prefix + " update should preserve selected-settings availability");
        expect_contains(update_process.stdout_text, "\"selectedReportSelectionKind\": \"settings\"",
                        issue_prefix + " update should preserve settings selection kind");
        expect_contains_in_order(
            update_process.stdout_text,
            {
                "\"deletedSettings\": [",
                "\"name\": \"ORIENTATION\", \"recordIndex\": 0, \"fieldIndex\": 2, \"sourceLineIndex\": 0",
                "\"value\": \"1\"",
                "\"name\": \"PAPERSIZE\", \"recordIndex\": 0, \"fieldIndex\": 2, \"sourceLineIndex\": 1",
                "\"value\": \"9\"",
                "\"name\": \"BOTMARGIN\", \"recordIndex\": 0, \"fieldIndex\": 2, \"sourceLineIndex\": 2",
                "\"value\": \"32\"",
                "\"name\": \"GRIDV\", \"recordIndex\": 0, \"fieldIndex\": 2, \"sourceLineIndex\": 3",
                "\"value\": \"6\"",
                "\"name\": \"GRIDH\", \"recordIndex\": 0, \"fieldIndex\": 2, \"sourceLineIndex\": 4",
                "\"value\": \"10\"",
                "\"name\": \"TOPMARGIN\", \"recordIndex\": 0, \"fieldIndex\": 2, \"sourceLineIndex\": 5",
                "\"value\": \"14\"",
                "\"name\": \"TOPMARGIN\", \"recordIndex\": 0, \"fieldIndex\": 8, \"sourceLineIndex\": null"
            },
            issue_prefix + " update should refresh deleted setting provenance");
        expect_contains_in_order(
            update_process.stdout_text,
            {
                "\"selectedReportSettings\": [",
                "\"name\": \"ORIENTATION\", \"recordIndex\": 0, \"fieldIndex\": 2, \"sourceLineIndex\": 0",
                "\"value\": \"1\"",
                "\"name\": \"PAPERSIZE\", \"recordIndex\": 0, \"fieldIndex\": 2, \"sourceLineIndex\": 1",
                "\"value\": \"9\"",
                "\"name\": \"BOTMARGIN\", \"recordIndex\": 0, \"fieldIndex\": 2, \"sourceLineIndex\": 2",
                "\"value\": \"32\"",
                "\"name\": \"GRIDV\", \"recordIndex\": 0, \"fieldIndex\": 2, \"sourceLineIndex\": 3",
                "\"value\": \"6\"",
                "\"name\": \"GRIDH\", \"recordIndex\": 0, \"fieldIndex\": 2, \"sourceLineIndex\": 4",
                "\"value\": \"10\"",
                "\"name\": \"TOPMARGIN\", \"recordIndex\": 0, \"fieldIndex\": 2, \"sourceLineIndex\": 5",
                "\"value\": \"14\"",
                "\"name\": \"TOPMARGIN\", \"recordIndex\": 0, \"fieldIndex\": 8, \"sourceLineIndex\": null"
            },
            issue_prefix + " update should refresh selected deleted settings");
        expect_contains(update_process.stdout_text, "\"sectionCount\": 2",
                        issue_prefix + " update should preserve live section metadata");
        expect_contains(update_process.stdout_text, "\"deletedObjectCount\": 1",
                        issue_prefix + " update should preserve deleted object metadata");
    }
}

void run_settings_memo_clear_case(
    const std::string& studio_host_path,
    const std::filesystem::path& temp_root,
    const std::string& asset_stem,
    const std::string& extension,
    bool deleted,
    const std::string& label,
    const std::string& issue_prefix) {
    const std::filesystem::path asset_path = temp_root / (asset_stem + extension);
    if (deleted) {
        write_synthetic_report_table_for_deleted_settings_json(asset_path);
    } else {
        write_synthetic_report_table_for_layout_json(asset_path);
    }

    const auto clear_process = run_process_capture(
        studio_host_path,
        {
            "--path", copperfin::test_support::path_to_utf8_string(asset_path),
            "--clear-property",
            "--record", "0",
            "--property-name", "EXPR",
            "--json"
        },
        temp_root);

    if (clear_process.exit_code != 0) {
        std::cerr << "studio host " << label << " settings memo clear " << extension << " stdout:\n"
                  << clear_process.stdout_text << "\n";
        std::cerr << "studio host " << label << " settings memo clear " << extension << " stderr:\n"
                  << clear_process.stderr_text << "\n";
        std::cerr << "fixture root: " << temp_root << "\n";
    }

    expect(clear_process.exit_code == 0, issue_prefix + " clear should exit successfully");
    const auto expr_property = copperfin::vfp::query_visual_object_property({
        .path = asset_path.string(),
        .record_index = 0U,
        .object_name = {},
        .unique_id = {},
        .property_name = "EXPR"
    });
    expect(expr_property.ok && expr_property.exists && expr_property.direct_field,
           issue_prefix + " clear should preserve the direct EXPR field carrier");
    expect_contains(clear_process.stdout_text, "\"documentTitle\": \"" + asset_path.filename().string() + "\"",
                    issue_prefix + " clear should return refreshed report-layout JSON");
    if (asset_path.extension() == ".lbx") {
        expect_contains(clear_process.stdout_text, "\"isLabel\": true",
                        issue_prefix + " clear should retain label identity");
    }
    expect_full_report_layout_preview_bounds(
        clear_process.stdout_text,
        deleted ? "#2038: record-selected deleted report/label settings memo clear JSON"
                : "#2038: record-selected report/label settings memo clear JSON");
    expect_cleared_settings_page_summary(clear_process.stdout_text, issue_prefix + " clear");
    if (!deleted) {
        expect_contains(clear_process.stdout_text, "\"settingCount\": 1",
                        issue_prefix + " clear should remove memo-derived settings from counts");
        expect_contains_in_order(
            clear_process.stdout_text,
            {
                "\"settings\": [",
                "\"name\": \"TOPMARGIN\", \"recordIndex\": 0, \"fieldIndex\": 8, \"sourceLineIndex\": null"
            },
            issue_prefix + " clear should preserve remaining direct-field provenance");
    } else {
        expect_contains(clear_process.stdout_text, "\"settingCount\": 0",
                        issue_prefix + " clear should not fabricate live settings");
        expect_contains(clear_process.stdout_text, "\"deletedSettingCount\": 1",
                        issue_prefix + " clear should refresh deleted setting counts");
        expect_contains(clear_process.stdout_text, "\"selectedReportSettingsAvailable\": true",
                        issue_prefix + " clear should preserve selected-settings availability");
        expect_contains(clear_process.stdout_text, "\"selectedReportSelectionKind\": \"settings\"",
                        issue_prefix + " clear should preserve settings selection kind");
        expect_contains_in_order(
            clear_process.stdout_text,
            {
                "\"deletedSettings\": [",
                "\"name\": \"TOPMARGIN\", \"recordIndex\": 0, \"fieldIndex\": 8, \"sourceLineIndex\": null"
            },
            issue_prefix + " clear should preserve remaining deleted direct-field provenance");
        expect_contains_in_order(
            clear_process.stdout_text,
            {
                "\"selectedReportSettings\": [",
                "\"name\": \"TOPMARGIN\", \"recordIndex\": 0, \"fieldIndex\": 8, \"sourceLineIndex\": null"
            },
            issue_prefix + " clear should refresh selected deleted settings");
        expect_not_contains(clear_process.stdout_text,
                            "\"name\": \"ORIENTATION\", \"recordIndex\": 0",
                            issue_prefix + " clear should remove memo-derived orientation settings");
        expect_contains(clear_process.stdout_text, "\"sectionCount\": 2",
                        issue_prefix + " clear should preserve live section metadata");
        expect_contains(clear_process.stdout_text, "\"deletedObjectCount\": 1",
                        issue_prefix + " clear should preserve deleted object metadata");
    }
}

void run_unsupported_settings_memo_update_case(
    const std::string& studio_host_path,
    const std::filesystem::path& temp_root,
    const std::string& asset_stem,
    const std::string& extension,
    const std::string& updated_settings,
    bool deleted,
    const std::string& label,
    const std::string& issue_prefix) {
    const std::filesystem::path asset_path = temp_root / (asset_stem + extension);
    if (deleted) {
        write_synthetic_report_table_for_deleted_unsupported_settings_json(asset_path);
    } else {
        write_synthetic_report_table_for_unsupported_settings_json(asset_path);
    }

    const auto update_process = run_process_capture(
        studio_host_path,
        {
            "--path", copperfin::test_support::path_to_utf8_string(asset_path),
            "--set-property",
            "--record", "0",
            "--property-name", "EXPR",
            "--property-value", updated_settings,
            "--json"
        },
        temp_root);

    if (update_process.exit_code != 0) {
        std::cerr << "studio host " << label << " unsupported settings memo update " << extension << " stdout:\n"
                  << update_process.stdout_text << "\n";
        std::cerr << "studio host " << label << " unsupported settings memo update " << extension << " stderr:\n"
                  << update_process.stderr_text << "\n";
        std::cerr << "fixture root: " << temp_root << "\n";
    }

    expect(update_process.exit_code == 0, issue_prefix + " update should exit successfully");
    const auto expr_property = copperfin::vfp::query_visual_object_property({
        .path = asset_path.string(),
        .record_index = 0U,
        .object_name = {},
        .unique_id = {},
        .property_name = "EXPR"
    });
    expect(expr_property.ok && expr_property.exists,
           issue_prefix + " update should leave the EXPR memo queryable");
    expect(normalize_line_endings(expr_property.value) == updated_settings,
           issue_prefix + " update should preserve the raw unsupported EXPR text");
    expect_updated_settings_page_summary(update_process.stdout_text, issue_prefix + " update");
    if (!deleted) {
        expect_contains(update_process.stdout_text, "\"settingCount\": 8",
                        issue_prefix + " update should expose parsed key/value settings only");
        expect_contains_in_order(
            update_process.stdout_text,
            {
                "\"settings\": [",
                "\"name\": \"ORIENTATION\", \"recordIndex\": 0, \"fieldIndex\": 2, \"sourceLineIndex\": 0",
                "\"name\": \"PAPERSIZE\", \"recordIndex\": 0, \"fieldIndex\": 2, \"sourceLineIndex\": 3",
                "\"name\": \"XUSER\", \"recordIndex\": 0, \"fieldIndex\": 2, \"sourceLineIndex\": 4",
                "\"name\": \"BOTMARGIN\", \"recordIndex\": 0, \"fieldIndex\": 2, \"sourceLineIndex\": 5",
                "\"name\": \"GRIDV\", \"recordIndex\": 0, \"fieldIndex\": 2, \"sourceLineIndex\": 6",
                "\"name\": \"GRIDH\", \"recordIndex\": 0, \"fieldIndex\": 2, \"sourceLineIndex\": 7",
                "\"name\": \"TOPMARGIN\", \"recordIndex\": 0, \"fieldIndex\": 2, \"sourceLineIndex\": 8",
                "\"name\": \"TOPMARGIN\", \"recordIndex\": 0, \"fieldIndex\": 8, \"sourceLineIndex\": null"
            },
            issue_prefix + " update should preserve source-line gaps around unsupported lines");
    } else {
        expect_contains(update_process.stdout_text, "\"settingCount\": 0",
                        issue_prefix + " update should not fabricate live settings");
        expect_contains(update_process.stdout_text, "\"deletedSettingCount\": 8",
                        issue_prefix + " update should expose deleted parsed key/value settings only");
        expect_contains_in_order(
            update_process.stdout_text,
            {
                "\"deletedSettings\": [",
                "\"name\": \"ORIENTATION\", \"recordIndex\": 0, \"fieldIndex\": 2, \"sourceLineIndex\": 0",
                "\"name\": \"PAPERSIZE\", \"recordIndex\": 0, \"fieldIndex\": 2, \"sourceLineIndex\": 3",
                "\"name\": \"XUSER\", \"recordIndex\": 0, \"fieldIndex\": 2, \"sourceLineIndex\": 4",
                "\"name\": \"BOTMARGIN\", \"recordIndex\": 0, \"fieldIndex\": 2, \"sourceLineIndex\": 5",
                "\"name\": \"GRIDV\", \"recordIndex\": 0, \"fieldIndex\": 2, \"sourceLineIndex\": 6",
                "\"name\": \"GRIDH\", \"recordIndex\": 0, \"fieldIndex\": 2, \"sourceLineIndex\": 7",
                "\"name\": \"TOPMARGIN\", \"recordIndex\": 0, \"fieldIndex\": 2, \"sourceLineIndex\": 8",
                "\"name\": \"TOPMARGIN\", \"recordIndex\": 0, \"fieldIndex\": 8, \"sourceLineIndex\": null"
            },
            issue_prefix + " update should preserve deleted source-line gaps around unsupported lines");
    }
    expect_contains(update_process.stdout_text, "\"selectedReportSettingsAvailable\": true",
                    issue_prefix + " update should preserve selected-settings availability");
    expect_contains(update_process.stdout_text, "\"selectedReportSelectionKind\": \"settings\"",
                    issue_prefix + " update should preserve settings selection kind");
    expect_contains_in_order(
        update_process.stdout_text,
        {
            "\"selectedReportSettings\": [",
            "\"name\": \"ORIENTATION\", \"recordIndex\": 0, \"fieldIndex\": 2, \"sourceLineIndex\": 0",
            "\"name\": \"PAPERSIZE\", \"recordIndex\": 0, \"fieldIndex\": 2, \"sourceLineIndex\": 3",
            "\"name\": \"XUSER\", \"recordIndex\": 0, \"fieldIndex\": 2, \"sourceLineIndex\": 4",
            "\"name\": \"BOTMARGIN\", \"recordIndex\": 0, \"fieldIndex\": 2, \"sourceLineIndex\": 5",
            "\"name\": \"GRIDV\", \"recordIndex\": 0, \"fieldIndex\": 2, \"sourceLineIndex\": 6",
            "\"name\": \"GRIDH\", \"recordIndex\": 0, \"fieldIndex\": 2, \"sourceLineIndex\": 7",
            "\"name\": \"TOPMARGIN\", \"recordIndex\": 0, \"fieldIndex\": 2, \"sourceLineIndex\": 8",
            "\"name\": \"TOPMARGIN\", \"recordIndex\": 0, \"fieldIndex\": 8, \"sourceLineIndex\": null"
        },
        issue_prefix + " update should refresh selected parsed settings without comment lines");
    expect_not_contains(update_process.stdout_text,
                        "\"name\": \"* keep-this-comment\"",
                        issue_prefix + " update should not fabricate comment lines as settings");
}

void run_unsupported_settings_memo_clear_case(
    const std::string& studio_host_path,
    const std::filesystem::path& temp_root,
    const std::string& asset_stem,
    const std::string& extension,
    bool deleted,
    const std::string& label,
    const std::string& issue_prefix) {
    const std::filesystem::path asset_path = temp_root / (asset_stem + extension);
    if (deleted) {
        write_synthetic_report_table_for_deleted_unsupported_settings_json(asset_path);
    } else {
        write_synthetic_report_table_for_unsupported_settings_json(asset_path);
    }

    const auto clear_process = run_process_capture(
        studio_host_path,
        {
            "--path", copperfin::test_support::path_to_utf8_string(asset_path),
            "--clear-property",
            "--record", "0",
            "--property-name", "EXPR",
            "--json"
        },
        temp_root);

    if (clear_process.exit_code != 0) {
        std::cerr << "studio host " << label << " unsupported settings memo clear " << extension << " stdout:\n"
                  << clear_process.stdout_text << "\n";
        std::cerr << "studio host " << label << " unsupported settings memo clear " << extension << " stderr:\n"
                  << clear_process.stderr_text << "\n";
        std::cerr << "fixture root: " << temp_root << "\n";
    }

    expect(clear_process.exit_code == 0, issue_prefix + " clear should exit successfully");
    const auto expr_property = copperfin::vfp::query_visual_object_property({
        .path = asset_path.string(),
        .record_index = 0U,
        .object_name = {},
        .unique_id = {},
        .property_name = "EXPR"
    });
    expect(expr_property.ok && expr_property.exists && expr_property.direct_field,
           issue_prefix + " clear should preserve the direct EXPR field carrier");
    expect_cleared_settings_page_summary(clear_process.stdout_text, issue_prefix + " clear");
    if (!deleted) {
        expect_contains(clear_process.stdout_text, "\"settingCount\": 1",
                        issue_prefix + " clear should remove memo-derived settings after unsupported input");
        expect_contains_in_order(
            clear_process.stdout_text,
            {
                "\"settings\": [",
                "\"name\": \"TOPMARGIN\", \"recordIndex\": 0, \"fieldIndex\": 8, \"sourceLineIndex\": null"
            },
            issue_prefix + " clear should preserve only the direct-field carrier");
    } else {
        expect_contains(clear_process.stdout_text, "\"settingCount\": 0",
                        issue_prefix + " clear should not fabricate live settings");
        expect_contains(clear_process.stdout_text, "\"deletedSettingCount\": 1",
                        issue_prefix + " clear should preserve only the deleted direct-field carrier");
        expect_contains_in_order(
            clear_process.stdout_text,
            {
                "\"deletedSettings\": [",
                "\"name\": \"TOPMARGIN\", \"recordIndex\": 0, \"fieldIndex\": 8, \"sourceLineIndex\": null"
            },
            issue_prefix + " clear should preserve only the deleted direct-field carrier");
    }
    expect_contains(clear_process.stdout_text, "\"selectedReportSettingsAvailable\": true",
                    issue_prefix + " clear should preserve selected-settings availability");
    expect_contains_in_order(
        clear_process.stdout_text,
        {
            "\"selectedReportSettings\": [",
            "\"name\": \"TOPMARGIN\", \"recordIndex\": 0, \"fieldIndex\": 8, \"sourceLineIndex\": null"
        },
        issue_prefix + " clear should preserve the selected direct-field carrier");
    expect_not_contains(clear_process.stdout_text,
                        "\"name\": \"* keep-this-comment\"",
                        issue_prefix + " clear should not fabricate comment lines as settings");
}

void test_studio_host_json_preserves_settings_memo_record_selection(const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_settings_memo_record_selection_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    ScopedDefaultLocaleCatalogEnvironment default_locale_environment;

    const std::string updated_settings =
        "ORIENTATION=1\n"
        "PAPERSIZE=9\n"
        "BOTMARGIN=32\n"
        "GRIDV=6\n"
        "GRIDH=10\n"
        "TOPMARGIN=14";
    const std::string unsupported_updated_settings =
        "ORIENTATION=1\n"
        "* keep-this-comment\n"
        "\n"
        "PAPERSIZE=9\n"
        "XUSER=keepme\n"
        "BOTMARGIN=32\n"
        "GRIDV=6\n"
        "GRIDH=10\n"
        "TOPMARGIN=14";

    run_settings_memo_update_case(
        studio_host_path,
        temp_root,
        "settings_memo",
        ".frx",
        updated_settings,
        false,
        "report",
        "#2784: report/label record-selected settings memo success");
    run_settings_memo_update_case(
        studio_host_path,
        temp_root,
        "settings_memo",
        ".lbx",
        updated_settings,
        false,
        "label",
        "#2784: report/label record-selected settings memo success");
    run_settings_memo_clear_case(
        studio_host_path,
        temp_root,
        "settings_memo_clear",
        ".frx",
        false,
        "report",
        "#2784: report/label record-selected settings memo success");
    run_settings_memo_clear_case(
        studio_host_path,
        temp_root,
        "settings_memo_clear",
        ".lbx",
        false,
        "label",
        "#2784: report/label record-selected settings memo success");
    run_settings_memo_update_case(
        studio_host_path,
        temp_root,
        "deleted_settings_memo",
        ".frx",
        updated_settings,
        true,
        "report",
        "#2784: report/label record-selected deleted settings memo success");
    run_settings_memo_update_case(
        studio_host_path,
        temp_root,
        "deleted_settings_memo",
        ".lbx",
        updated_settings,
        true,
        "label",
        "#2784: report/label record-selected deleted settings memo success");
    run_settings_memo_clear_case(
        studio_host_path,
        temp_root,
        "deleted_settings_memo_clear",
        ".frx",
        true,
        "report",
        "#2784: report/label record-selected deleted settings memo success");
    run_settings_memo_clear_case(
        studio_host_path,
        temp_root,
        "deleted_settings_memo_clear",
        ".lbx",
        true,
        "label",
        "#2784: report/label record-selected deleted settings memo success");
    run_unsupported_settings_memo_update_case(
        studio_host_path,
        temp_root,
        "unsupported_settings_memo",
        ".frx",
        unsupported_updated_settings,
        false,
        "report",
        "#3097: report/label record-selected settings memo should handle unsupported lines");
    run_unsupported_settings_memo_update_case(
        studio_host_path,
        temp_root,
        "unsupported_settings_memo",
        ".lbx",
        unsupported_updated_settings,
        false,
        "label",
        "#3097: report/label record-selected settings memo should handle unsupported lines");
    run_unsupported_settings_memo_clear_case(
        studio_host_path,
        temp_root,
        "unsupported_settings_memo_clear",
        ".frx",
        false,
        "report",
        "#3097: report/label record-selected settings memo clear should handle unsupported lines");
    run_unsupported_settings_memo_clear_case(
        studio_host_path,
        temp_root,
        "unsupported_settings_memo_clear",
        ".lbx",
        false,
        "label",
        "#3097: report/label record-selected settings memo clear should handle unsupported lines");
    run_unsupported_settings_memo_update_case(
        studio_host_path,
        temp_root,
        "deleted_unsupported_settings_memo",
        ".frx",
        unsupported_updated_settings,
        true,
        "report",
        "#3097: report/label record-selected deleted settings memo should handle unsupported lines");
    run_unsupported_settings_memo_update_case(
        studio_host_path,
        temp_root,
        "deleted_unsupported_settings_memo",
        ".lbx",
        unsupported_updated_settings,
        true,
        "label",
        "#3097: report/label record-selected deleted settings memo should handle unsupported lines");
    run_unsupported_settings_memo_clear_case(
        studio_host_path,
        temp_root,
        "deleted_unsupported_settings_memo_clear",
        ".frx",
        true,
        "report",
        "#3097: report/label record-selected deleted settings memo clear should handle unsupported lines");
    run_unsupported_settings_memo_clear_case(
        studio_host_path,
        temp_root,
        "deleted_unsupported_settings_memo_clear",
        ".lbx",
        true,
        "label",
        "#3097: report/label record-selected deleted settings memo clear should handle unsupported lines");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

}  // namespace

int main(int argc, char** argv) {
    if (argc < 2) {
        std::cerr << "Usage: test_studio_host_settings_memo_record <studio_host_path>\n";
        return 1;
    }

    test_studio_host_json_preserves_settings_memo_record_selection(argv[1]);

    if (failures != 0) {
        std::cerr << failures << " test(s) failed.\n";
        return 1;
    }

    std::cout << "All tests passed.\n";
    return 0;
}
