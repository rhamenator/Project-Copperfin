// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only

#include "test_report_layout_support.h"

namespace cf_test_report_layout {

void test_report_header_unique_provenance() {
    copperfin::studio::StudioDocumentModel document;
    document.display_name = "header-unique.frx";
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
                value("CURPOS", "T", 713U),
                value("UNIQUE", "T", 717U),
                value("ORDER", "ORDER-BYTES", 721U),
                value("COMMENT", "Header developer note", 722U),
                value("USER", "Header user comment", 723U)
            }
        },
        {
            .record_index = 1U,
            .deleted = true,
            .values = {
                value("OBJTYPE", "1"),
                value("UNIQUE", "", 718U)
            }
        }
    };

    const auto layout = copperfin::studio::build_report_layout(document);
    expect(layout.available, "#4542: UNIQUE fixture should produce a report layout snapshot");

    const auto find_setting = [](const auto& settings, std::string_view name) {
        return std::find_if(
            settings.begin(),
            settings.end(),
            [name](const auto& setting) { return setting.name == name; });
    };

    const auto unique = find_setting(layout.settings, "UNIQUE");
    expect(unique != layout.settings.end() && unique->value == "T" &&
           unique->field_index == 4U && unique->memo_block_number == 717U,
        "#4542: live header UNIQUE should preserve value and provenance");
    expect(find_setting(layout.deleted_settings, "UNIQUE") == layout.deleted_settings.end(),
        "#4542: blank deleted UNIQUE should not fabricate a setting");
    const auto order = find_setting(layout.settings, "ORDER");
    expect(order != layout.settings.end() && order->value == "hex:4F524445522D4259544553" &&
           order->field_index == 5U && order->memo_block_number == 721U,
        "#4543: binary ORDER should use a stable hex value while preserving provenance");
    expect(layout.settings.size() == 7U,
        "#4543/#4544/#4545: ORDER, COMMENT, and USER should append without changing earlier header setting order");
    const auto comment = find_setting(layout.settings, "COMMENT");
    expect(comment != layout.settings.end() && comment->value == "Header developer note" &&
           comment->field_index == 6U && comment->memo_block_number == 722U,
        "#4544: live header COMMENT should preserve value and memo provenance");
    const auto user_comment = find_setting(layout.settings, "USER");
    expect(user_comment != layout.settings.end() && user_comment->value == "Header user comment" &&
           user_comment->field_index == 7U && user_comment->memo_block_number == 723U,
        "#4545: live header USER should preserve value and memo provenance");
}

}  // namespace cf_test_report_layout
