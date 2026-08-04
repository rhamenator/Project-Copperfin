// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only

#include "test_report_layout_support.h"

namespace cf_test_report_layout {

void test_report_header_view_settings_provenance() {
    copperfin::studio::StudioDocumentModel document;
    document.display_name = "header-view.frx";
    document.kind = copperfin::studio::StudioAssetKind::report;
    document.table_preview_available = true;
    document.table_preview.records = {
        {
            .record_index = 0U,
            .deleted = false,
            .values = {
                value("OBJTYPE", "1"),
                value("GRID", "T", 700U),
                value("RULER", "4", 701U)
            }
        },
        {
            .record_index = 1U,
            .deleted = true,
            .values = {
                value("OBJTYPE", "1"),
                value("GRID", "", 702U),
                value("RULER", "2", 703U)
            }
        }
    };

    const auto layout = copperfin::studio::build_report_layout(document);
    expect(layout.available, "#4537: header view fixture should produce a report layout snapshot");

    const auto find_setting = [](const auto& settings, std::string_view name) {
        return std::find_if(
            settings.begin(),
            settings.end(),
            [name](const auto& setting) { return setting.name == name; });
    };

    const auto grid = find_setting(layout.settings, "GRID");
    const auto ruler = find_setting(layout.settings, "RULER");
    expect(grid != layout.settings.end() && grid->value == "T" &&
           grid->field_index == 1U && grid->memo_block_number == 700U,
        "#4537: live header GRID should preserve value and provenance");
    expect(ruler != layout.settings.end() && ruler->value == "4" &&
           ruler->field_index == 2U && ruler->memo_block_number == 701U,
        "#4537: live header RULER should preserve value and provenance");

    const auto deleted_grid = find_setting(layout.deleted_settings, "GRID");
    const auto deleted_ruler = find_setting(layout.deleted_settings, "RULER");
    expect(deleted_grid == layout.deleted_settings.end(),
        "#4537: blank deleted GRID should not fabricate a setting");
    expect(deleted_ruler != layout.deleted_settings.end() && deleted_ruler->value == "2" &&
           deleted_ruler->field_index == 2U && deleted_ruler->memo_block_number == 703U,
        "#4537: deleted header RULER should preserve value and provenance");
}

}  // namespace cf_test_report_layout
