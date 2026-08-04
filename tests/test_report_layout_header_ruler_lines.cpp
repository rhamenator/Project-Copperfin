// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only

#include "test_report_layout_support.h"

namespace cf_test_report_layout {

void test_report_header_ruler_lines_provenance() {
    copperfin::studio::StudioDocumentModel document;
    document.display_name = "header-ruler-lines.frx";
    document.kind = copperfin::studio::StudioAssetKind::report;
    document.table_preview_available = true;
    document.table_preview.records = {
        {
            .record_index = 0U,
            .deleted = false,
            .values = {
                value("OBJTYPE", "1"),
                value("RULERLINES", "1", 704U)
            }
        },
        {
            .record_index = 1U,
            .deleted = false,
            .values = {
                value("OBJTYPE", "8"),
                value("RULERLINES", "4", 705U)
            }
        },
        {
            .record_index = 2U,
            .deleted = true,
            .values = {
                value("OBJTYPE", "1"),
                value("RULERLINES", "", 706U)
            }
        }
    };

    const auto layout = copperfin::studio::build_report_layout(document);
    expect(layout.available, "#4538: header ruler-line fixture should produce a report layout snapshot");

    const auto find_setting = [](const auto& settings, std::string_view name) {
        return std::find_if(
            settings.begin(),
            settings.end(),
            [name](const auto& setting) { return setting.name == name; });
    };

    const auto header_setting = find_setting(layout.settings, "RULERLINES");
    expect(header_setting != layout.settings.end() && header_setting->value == "1" &&
           header_setting->field_index == 1U && header_setting->memo_block_number == 704U,
        "#4538: header RULERLINES should preserve value and provenance");

    expect(layout.unplaced_objects.size() == 1U &&
           layout.unplaced_objects.front().objtype_code == 8,
        "#4538: expression fixture should remain an unplaced expression object");
    const auto expression_highlight = find_setting(layout.unplaced_objects.front().highlights, "RULERLINES");
    expect(expression_highlight != layout.unplaced_objects.front().highlights.end() &&
           expression_highlight->value == "4" && expression_highlight->field_index == 1U &&
           expression_highlight->memo_block_number == 705U,
        "#4538: expression-object RULERLINES should remain an object highlight with provenance");
    expect(layout.settings.size() == 1U,
        "#4538: expression-object RULERLINES should not enter header settings");

    const auto deleted_setting = find_setting(layout.deleted_settings, "RULERLINES");
    expect(deleted_setting == layout.deleted_settings.end(),
        "#4538: blank deleted header RULERLINES should not fabricate a setting");
}

}  // namespace cf_test_report_layout
