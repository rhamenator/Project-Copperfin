// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.

#include "copperfin/studio/report_layout.h"

#include <algorithm>
#include <cstdint>
#include <iostream>
#include <string>
#include <utility>
#include <vector>

namespace {

int failures = 0;

void expect(bool condition, const std::string& message) {
    if (!condition) {
        std::cerr << "FAIL: " << message << "\n";
        ++failures;
    }
}

copperfin::vfp::DbfRecordValue value(
    std::string name,
    std::string display_value,
    std::uint32_t memo_block_number = 0U) {
    return {
        .field_name = std::move(name),
        .field_type = 'C',
        .is_null = false,
        .display_value = std::move(display_value),
        .memo_block_number = memo_block_number
    };
}

void test_build_report_layout_includes_direct_side_margin_settings() {
    copperfin::studio::StudioDocumentModel document;
    document.display_name = "side-margins.frx";
    document.kind = copperfin::studio::StudioAssetKind::report;
    document.table_preview_available = true;

    document.table_preview.records = {
        {
            .record_index = 0U,
            .deleted = false,
            .values = {
                value("OBJTYPE", "1"),
                value("OBJCODE", "53"),
                value("EXPR", "ORIENTATION=1\r\nPAPERSIZE=9", 44U),
                value("LEFTMARGIN", "15"),
                value("RIGHTMARGIN", "25")
            }
        }
    };

    const auto layout = copperfin::studio::build_report_layout(document);
    expect(layout.available, "#3014: report layout should stay available for direct side-margin settings");
    expect(layout.page_setup_available,
           "#3014: direct side-margin settings should still mark page setup available");
    expect(layout.settings.size() == 4U,
           "#3014: direct side-margin settings should be counted alongside memo-derived settings");

    const auto left_margin = std::find_if(layout.settings.begin(), layout.settings.end(), [](const auto& setting) {
        return setting.name == "LEFTMARGIN";
    });
    expect(left_margin != layout.settings.end(),
           "#3014: direct left-margin settings should surface in live report settings");
    if (left_margin != layout.settings.end()) {
        expect(left_margin->record_index == 0U,
               "#3014: direct left-margin settings should retain root record provenance");
        expect(left_margin->field_index == 3U,
               "#3014: direct left-margin settings should retain DBF field provenance");
        expect(left_margin->source_line_index == copperfin::studio::StudioReportMissingLineIndex,
               "#3014: direct left-margin settings should not masquerade as memo-line settings");
        expect(left_margin->memo_block_number == 0U,
               "#3014: direct left-margin settings should expose memo block zero for non-memo-backed roots");
        expect(left_margin->value == "15",
               "#3014: direct left-margin settings should preserve side-margin values");
    }

    const auto right_margin = std::find_if(layout.settings.begin(), layout.settings.end(), [](const auto& setting) {
        return setting.name == "RIGHTMARGIN";
    });
    expect(right_margin != layout.settings.end(),
           "#3014: direct right-margin settings should surface in live report settings");
    if (right_margin != layout.settings.end()) {
        expect(right_margin->record_index == 0U,
               "#3014: direct right-margin settings should retain root record provenance");
        expect(right_margin->field_index == 4U,
               "#3014: direct right-margin settings should retain DBF field provenance");
        expect(right_margin->source_line_index == copperfin::studio::StudioReportMissingLineIndex,
               "#3014: direct right-margin settings should not masquerade as memo-line settings");
        expect(right_margin->memo_block_number == 0U,
               "#3014: direct right-margin settings should expose memo block zero for non-memo-backed roots");
        expect(right_margin->value == "25",
               "#3014: direct right-margin settings should preserve side-margin values");
    }
}

void test_build_report_layout_includes_deleted_direct_side_margin_settings() {
    copperfin::studio::StudioDocumentModel document;
    document.display_name = "deleted-side-margins.frx";
    document.kind = copperfin::studio::StudioAssetKind::report;
    document.table_preview_available = true;

    document.table_preview.records = {
        {
            .record_index = 0U,
            .deleted = true,
            .values = {
                value("OBJTYPE", "1"),
                value("OBJCODE", "53"),
                value("EXPR", "ORIENTATION=1", 45U),
                value("LEFTMARGIN", "35"),
                value("RIGHTMARGIN", "45")
            }
        }
    };

    const auto layout = copperfin::studio::build_report_layout(document);
    expect(layout.available,
           "#3014: report layout should stay available for deleted direct side-margin settings");
    expect(layout.settings.empty(),
           "#3014: deleted side-margin settings should not leak into live report settings");
    expect(layout.deleted_settings.size() == 3U,
           "#3014: deleted side-margin settings should be counted with deleted report settings");

    const auto left_margin = std::find_if(layout.deleted_settings.begin(), layout.deleted_settings.end(), [](const auto& setting) {
        return setting.name == "LEFTMARGIN";
    });
    expect(left_margin != layout.deleted_settings.end(),
           "#3014: deleted left-margin settings should surface in deleted report settings");
    if (left_margin != layout.deleted_settings.end()) {
        expect(left_margin->record_index == 0U,
               "#3014: deleted left-margin settings should retain root record provenance");
        expect(left_margin->field_index == 3U,
               "#3014: deleted left-margin settings should retain DBF field provenance");
        expect(left_margin->source_line_index == copperfin::studio::StudioReportMissingLineIndex,
               "#3014: deleted left-margin settings should not masquerade as memo-line settings");
        expect(left_margin->memo_block_number == 0U,
               "#3014: deleted left-margin settings should expose memo block zero for non-memo-backed roots");
        expect(left_margin->value == "35",
               "#3014: deleted left-margin settings should preserve deleted side-margin values");
    }

    const auto right_margin = std::find_if(layout.deleted_settings.begin(), layout.deleted_settings.end(), [](const auto& setting) {
        return setting.name == "RIGHTMARGIN";
    });
    expect(right_margin != layout.deleted_settings.end(),
           "#3014: deleted right-margin settings should surface in deleted report settings");
    if (right_margin != layout.deleted_settings.end()) {
        expect(right_margin->record_index == 0U,
               "#3014: deleted right-margin settings should retain root record provenance");
        expect(right_margin->field_index == 4U,
               "#3014: deleted right-margin settings should retain DBF field provenance");
        expect(right_margin->source_line_index == copperfin::studio::StudioReportMissingLineIndex,
               "#3014: deleted right-margin settings should not masquerade as memo-line settings");
        expect(right_margin->memo_block_number == 0U,
               "#3014: deleted right-margin settings should expose memo block zero for non-memo-backed roots");
        expect(right_margin->value == "45",
               "#3014: deleted right-margin settings should preserve deleted side-margin values");
    }
}

void test_build_report_layout_ignores_blank_direct_side_margin_settings() {
    copperfin::studio::StudioDocumentModel document;
    document.display_name = "blank-side-margins.frx";
    document.kind = copperfin::studio::StudioAssetKind::report;
    document.table_preview_available = true;

    document.table_preview.records = {
        {
            .record_index = 0U,
            .deleted = false,
            .values = {
                value("OBJTYPE", "1"),
                value("OBJCODE", "53"),
                value("EXPR", "ORIENTATION=1", 46U),
                value("LEFTMARGIN", "   "),
                value("RIGHTMARGIN", "")
            }
        }
    };

    const auto layout = copperfin::studio::build_report_layout(document);
    const auto left_margin = std::find_if(layout.settings.begin(), layout.settings.end(), [](const auto& setting) {
        return setting.name == "LEFTMARGIN";
    });
    expect(left_margin == layout.settings.end(),
           "#3014: blank LEFTMARGIN settings should stay absent like other blank report settings");

    const auto right_margin = std::find_if(layout.settings.begin(), layout.settings.end(), [](const auto& setting) {
        return setting.name == "RIGHTMARGIN";
    });
    expect(right_margin == layout.settings.end(),
           "#3014: blank RIGHTMARGIN settings should stay absent like other blank report settings");
}

}  // namespace

int main() {
    test_build_report_layout_includes_direct_side_margin_settings();
    test_build_report_layout_includes_deleted_direct_side_margin_settings();
    test_build_report_layout_ignores_blank_direct_side_margin_settings();

    if (failures != 0) {
        std::cerr << failures << " test(s) failed.\n";
        return 1;
    }

    std::cout << "All tests passed.\n";
    return 0;
}
