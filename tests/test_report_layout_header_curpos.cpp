// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.

#include "test_report_layout_support.h"

namespace cf_test_report_layout {

void test_report_header_curpos_provenance() {
    copperfin::studio::StudioDocumentModel document;
    document.display_name = "header-curpos.frx";
    document.kind = copperfin::studio::StudioAssetKind::report;
    document.table_preview_available = true;
    document.table_preview.records = {
        {
            .record_index = 0U,
            .deleted = false,
            .values = {
                value("OBJTYPE", "1"),
                value("RULERLINES", "1", 704U),
                value("ADDALIAS", "T", 709U),
                value("CURPOS", "T", 713U)
            }
        },
        {
            .record_index = 1U,
            .deleted = true,
            .values = {
                value("OBJTYPE", "1"),
                value("CURPOS", "", 714U)
            }
        }
    };

    const auto layout = copperfin::studio::build_report_layout(document);
    expect(layout.available, "#4541: CURPOS fixture should produce a report layout snapshot");

    const auto find_setting = [](const auto& settings, std::string_view name) {
        return std::find_if(
            settings.begin(),
            settings.end(),
            [name](const auto& setting) { return setting.name == name; });
    };

    const auto curpos = find_setting(layout.settings, "CURPOS");
    expect(curpos != layout.settings.end() && curpos->value == "T" &&
           curpos->field_index == 3U && curpos->memo_block_number == 713U,
        "#4541: live header CURPOS should preserve value and provenance");
    expect(find_setting(layout.deleted_settings, "CURPOS") == layout.deleted_settings.end(),
        "#4541: blank deleted CURPOS should not fabricate a setting");
    expect(layout.settings.size() == 3U,
        "#4541: CURPOS should append without changing earlier header setting order");
}

}  // namespace cf_test_report_layout
