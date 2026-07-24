// Copyright © 2026 Richard M. Hamilton. All rights reserved.
// Licensed under the Project Copperfin Source-Available License or
// Commercial License. See LICENSE.md in the repository root.

#include "test_report_layout_support.h"

namespace cf_test_report_layout {

void test_report_line_shape_style_provenance() {
    copperfin::studio::StudioDocumentModel document;
    document.display_name = "line-shape-style.frx";
    document.kind = copperfin::studio::StudioAssetKind::report;
    document.table_preview_available = true;
    document.table_preview.records = {
        {
            .record_index = 0U,
            .deleted = false,
            .values = {
                value("OBJTYPE", "1"),
                value("OBJCODE", "53")
            }
        },
        {
            .record_index = 1U,
            .deleted = false,
            .values = {
                value("OBJTYPE", "9"),
                value("OBJCODE", "4"),
                value("VPOS", "0.000"),
                value("HEIGHT", "3000.000")
            }
        },
        {
            .record_index = 2U,
            .deleted = false,
            .values = {
                value("OBJTYPE", "6"),
                value("PENSIZE", "2", 710U),
                value("PENPAT", "3", 711U),
                value("FILLPAT", "6", 712U),
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
                value("OBJTYPE", "7"),
                value("PENSIZE", "4", 713U),
                value("PENPAT", "8", 714U),
                value("FILLPAT", "2", 715U),
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
                value("OBJTYPE", "6"),
                value("PENSIZE", "", 716U),
                value("PENPAT", "", 717U),
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
                value("OBJTYPE", "5"),
                value("PENSIZE", "6", 718U),
                value("PENPAT", "7", 719U),
                value("FILLPAT", "1", 720U),
                value("HPOS", "2800.000"),
                value("VPOS", "200.000"),
                value("WIDTH", "800.000"),
                value("HEIGHT", "400.000")
            }
        }
    };

    const auto layout = copperfin::studio::build_report_layout(document);
    expect(layout.available && layout.sections.size() == 1U,
        "#4535: line and shape style fixture should produce one live band");

    const auto find_live = [&](std::size_t record_index) {
        return std::find_if(
            layout.sections[0].objects.begin(),
            layout.sections[0].objects.end(),
            [record_index](const auto& object) {
                return object.record_index == record_index;
            });
    };
    const auto find_highlight = [](const auto& object, std::string_view name) {
        return std::find_if(
            object.highlights.begin(),
            object.highlights.end(),
            [name](const auto& highlight) {
                return highlight.name == name;
            });
    };

    if (!layout.sections.empty()) {
        const auto line = find_live(2U);
        expect(line != layout.sections[0].objects.end(),
            "#4535: live line objects should remain selectable");
        if (line != layout.sections[0].objects.end()) {
            const auto pen_size = find_highlight(*line, "PENSIZE");
            const auto pen_pattern = find_highlight(*line, "PENPAT");
            expect(pen_size != line->highlights.end() &&
                   pen_size->value == "2" && pen_size->field_index == 1U &&
                   pen_size->memo_block_number == 710U,
                "#4535: line PENSIZE should preserve value and provenance");
            expect(pen_pattern != line->highlights.end() &&
                   pen_pattern->value == "3" && pen_pattern->field_index == 2U &&
                   pen_pattern->memo_block_number == 711U,
                "#4535: line PENPAT should preserve value and provenance");
            expect(find_highlight(*line, "FILLPAT") == line->highlights.end(),
                "#4535: line objects should not receive shape-only FILLPAT");
        }

        const auto blank = find_live(4U);
        expect(blank != layout.sections[0].objects.end(),
            "#4535: blank line objects should remain selectable");
        if (blank != layout.sections[0].objects.end()) {
            expect(find_highlight(*blank, "PENSIZE") == blank->highlights.end() &&
                   find_highlight(*blank, "PENPAT") == blank->highlights.end(),
                "#4535: blank line style values should not fabricate highlights");
        }

        const auto label = find_live(5U);
        expect(label != layout.sections[0].objects.end(),
            "#4535: unrelated label objects should remain selectable");
        if (label != layout.sections[0].objects.end()) {
            expect(find_highlight(*label, "PENSIZE") == label->highlights.end() &&
                   find_highlight(*label, "PENPAT") == label->highlights.end() &&
                   find_highlight(*label, "FILLPAT") == label->highlights.end(),
                "#4535: label objects should not receive line and shape style fields");
        }
    }

    const auto deleted = std::find_if(
        layout.deleted_objects.begin(),
        layout.deleted_objects.end(),
        [](const auto& object) {
            return object.record_index == 3U;
        });
    expect(deleted != layout.deleted_objects.end(),
        "#4535: deleted shape objects should remain available for inspection");
    if (deleted != layout.deleted_objects.end()) {
        const auto pen_size = find_highlight(*deleted, "PENSIZE");
        const auto pen_pattern = find_highlight(*deleted, "PENPAT");
        const auto fill_pattern = find_highlight(*deleted, "FILLPAT");
        expect(pen_size != deleted->highlights.end() &&
               pen_size->value == "4" && pen_size->field_index == 1U &&
               pen_size->memo_block_number == 713U,
            "#4535: deleted shape PENSIZE should preserve value and provenance");
        expect(pen_pattern != deleted->highlights.end() &&
               pen_pattern->value == "8" && pen_pattern->field_index == 2U &&
               pen_pattern->memo_block_number == 714U,
            "#4535: deleted shape PENPAT should preserve value and provenance");
        expect(fill_pattern != deleted->highlights.end() &&
               fill_pattern->value == "2" && fill_pattern->field_index == 3U &&
               fill_pattern->memo_block_number == 715U,
            "#4535: deleted shape FILLPAT should preserve value and provenance");
    }
}

}  // namespace cf_test_report_layout
