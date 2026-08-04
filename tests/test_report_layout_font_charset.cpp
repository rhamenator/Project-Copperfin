// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only

#include "test_report_layout_support.h"

namespace cf_test_report_layout {

void test_report_font_charset_provenance() {
    copperfin::studio::StudioDocumentModel document;
    document.display_name = "font-charset.frx";
    document.kind = copperfin::studio::StudioAssetKind::report;
    document.table_preview_available = true;
    document.table_preview.records = {
        {
            .record_index = 0U,
            .deleted = false,
            .values = {
                value("OBJTYPE", "1"),
                value("DOUBLE", "T", 700U),
                value("RESOID", "0", 701U)
            }
        },
        {
            .record_index = 1U,
            .deleted = false,
            .values = {
                value("OBJTYPE", "9"),
                value("VPOS", "0.000"),
                value("HEIGHT", "3000.000")
            }
        },
        {
            .record_index = 2U,
            .deleted = false,
            .values = {
                value("OBJTYPE", "5"),
                value("DOUBLE", "T", 710U),
                value("RESOID", "1", 711U),
                value("HPOS", "100.000"),
                value("VPOS", "200.000"),
                value("WIDTH", "800.000"),
                value("HEIGHT", "400.000")
            }
        },
        {
            .record_index = 3U,
            .deleted = true,
            .values = {
                value("OBJTYPE", "8"),
                value("DOUBLE", "F", 712U),
                value("RESOID", "2", 713U),
                value("HPOS", "1000.000"),
                value("VPOS", "200.000"),
                value("WIDTH", "800.000"),
                value("HEIGHT", "400.000")
            }
        },
        {
            .record_index = 4U,
            .deleted = false,
            .values = {
                value("OBJTYPE", "17"),
                value("DOUBLE", "T", 714U),
                value("RESOID", "3", 715U),
                value("HPOS", "1900.000"),
                value("VPOS", "200.000"),
                value("WIDTH", "800.000"),
                value("HEIGHT", "400.000")
            }
        },
        {
            .record_index = 5U,
            .deleted = false,
            .values = {
                value("OBJTYPE", "6"),
                value("DOUBLE", "T", 716U),
                value("RESOID", "4", 717U),
                value("HPOS", "2800.000"),
                value("VPOS", "200.000"),
                value("WIDTH", "800.000"),
                value("HEIGHT", "400.000")
            }
        },
        {
            .record_index = 6U,
            .deleted = false,
            .values = {
                value("OBJTYPE", "5"),
                value("DOUBLE", "", 718U),
                value("RESOID", "", 719U),
                value("HPOS", "3700.000"),
                value("VPOS", "200.000"),
                value("WIDTH", "800.000"),
                value("HEIGHT", "400.000")
            }
        }
    };

    const auto layout = copperfin::studio::build_report_layout(document);
    expect(layout.available && layout.sections.size() == 1U,
        "#4536: font-charset fixture should produce one live band");
    const auto find_setting = [&](std::string_view name) {
        return std::find_if(
            layout.settings.begin(),
            layout.settings.end(),
            [name](const auto& setting) { return setting.name == name; });
    };
    const auto explicit_header_charset = find_setting("DOUBLE");
    const auto header_charset = find_setting("RESOID");
    expect(explicit_header_charset != layout.settings.end() && explicit_header_charset->value == "T" &&
           explicit_header_charset->field_index == 1U && explicit_header_charset->memo_block_number == 700U,
        "#4536: header DOUBLE should preserve value and provenance");
    expect(header_charset != layout.settings.end() && header_charset->value == "0" &&
           header_charset->field_index == 2U && header_charset->memo_block_number == 701U,
        "#4536: header RESOID should preserve value and provenance");

    const auto find_highlight = [](const auto& object, std::string_view name) {
        return std::find_if(
            object.highlights.begin(),
            object.highlights.end(),
            [name](const auto& highlight) { return highlight.name == name; });
    };
    const auto find_live = [&](std::size_t record_index) {
        return std::find_if(
            layout.sections[0].objects.begin(),
            layout.sections[0].objects.end(),
            [record_index](const auto& object) { return object.record_index == record_index; });
    };

    if (!layout.sections.empty()) {
        const auto label = find_live(2U);
        expect(label != layout.sections[0].objects.end(),
            "#4536: live labels should remain selectable");
        if (label != layout.sections[0].objects.end()) {
            const auto explicit_charset = find_highlight(*label, "DOUBLE");
            const auto charset = find_highlight(*label, "RESOID");
            expect(explicit_charset != label->highlights.end() && explicit_charset->value == "T" &&
                   explicit_charset->field_index == 1U && explicit_charset->memo_block_number == 710U,
                "#4536: label DOUBLE should preserve value and provenance");
            expect(charset != label->highlights.end() && charset->value == "1" &&
                   charset->field_index == 2U && charset->memo_block_number == 711U,
                "#4536: label RESOID should preserve value and provenance");
        }

        const auto image = find_live(4U);
        expect(image != layout.sections[0].objects.end(),
            "#4536: live pictures should remain selectable");
        if (image != layout.sections[0].objects.end()) {
            expect(find_highlight(*image, "DOUBLE") != image->highlights.end(),
                "#4536: pictures should expose DOUBLE");
            expect(find_highlight(*image, "RESOID") == image->highlights.end(),
                "#4536: pictures should not expose RESOID");
        }

        const auto line = find_live(5U);
        expect(line != layout.sections[0].objects.end(),
            "#4536: unrelated line objects should remain selectable");
        if (line != layout.sections[0].objects.end()) {
            expect(find_highlight(*line, "DOUBLE") == line->highlights.end() &&
                   find_highlight(*line, "RESOID") == line->highlights.end(),
                "#4536: line objects should not receive font-charset fields");
        }

        const auto blank = find_live(6U);
        expect(blank != layout.sections[0].objects.end(),
            "#4536: blank labels should remain selectable");
        if (blank != layout.sections[0].objects.end()) {
            expect(find_highlight(*blank, "DOUBLE") == blank->highlights.end() &&
                   find_highlight(*blank, "RESOID") == blank->highlights.end(),
                "#4536: blank font-charset values should not fabricate highlights");
        }
    }

    const auto deleted = std::find_if(
        layout.deleted_objects.begin(),
        layout.deleted_objects.end(),
        [](const auto& object) { return object.record_index == 3U; });
    expect(deleted != layout.deleted_objects.end(),
        "#4536: deleted expression objects should remain available for inspection");
    if (deleted != layout.deleted_objects.end()) {
        const auto explicit_charset = find_highlight(*deleted, "DOUBLE");
        const auto charset = find_highlight(*deleted, "RESOID");
        expect(explicit_charset != deleted->highlights.end() && explicit_charset->value == "F" &&
               explicit_charset->field_index == 1U && explicit_charset->memo_block_number == 712U,
            "#4536: deleted expression DOUBLE should preserve value and provenance");
        expect(charset != deleted->highlights.end() && charset->value == "2" &&
               charset->field_index == 2U && charset->memo_block_number == 713U,
            "#4536: deleted expression RESOID should preserve value and provenance");
    }
}

}  // namespace cf_test_report_layout
