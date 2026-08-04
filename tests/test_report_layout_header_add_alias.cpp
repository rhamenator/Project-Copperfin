// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only

#include "test_report_layout_support.h"

namespace cf_test_report_layout {

void test_report_header_add_alias_provenance() {
    copperfin::studio::StudioDocumentModel document;
    document.display_name = "header-add-alias.frx";
    document.kind = copperfin::studio::StudioAssetKind::report;
    document.table_preview_available = true;
    document.table_preview.records = {
        {
            .record_index = 0U,
            .deleted = false,
            .values = {
                value("OBJTYPE", "1"),
                value("RULERLINES", "1", 704U),
                value("ADDALIAS", "T", 709U)
            }
        },
        {
            .record_index = 1U,
            .deleted = true,
            .values = {
                value("OBJTYPE", "1"),
                value("ADDALIAS", "", 710U)
            }
        }
    };

    const auto layout = copperfin::studio::build_report_layout(document);
    expect(layout.available, "#4540: ADDALIAS fixture should produce a report layout snapshot");

    const auto find_setting = [](const auto& settings, std::string_view name) {
        return std::find_if(
            settings.begin(),
            settings.end(),
            [name](const auto& setting) { return setting.name == name; });
    };

    const auto add_alias = find_setting(layout.settings, "ADDALIAS");
    expect(add_alias != layout.settings.end() && add_alias->value == "T" &&
           add_alias->field_index == 2U && add_alias->memo_block_number == 709U,
        "#4540: live header ADDALIAS should preserve value and provenance");
    expect(find_setting(layout.deleted_settings, "ADDALIAS") == layout.deleted_settings.end(),
        "#4540: blank deleted ADDALIAS should not fabricate a setting");
    expect(layout.settings.size() == 2U,
        "#4540: ADDALIAS should append without changing earlier header setting order");
}

}  // namespace cf_test_report_layout
