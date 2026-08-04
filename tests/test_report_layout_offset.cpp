// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only

#include "test_report_layout_support.h"

namespace cf_test_report_layout {

void test_report_expression_offset_provenance() {
    copperfin::studio::StudioDocumentModel document;
    document.display_name = "offset.frx";
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
                value("HEIGHT", "2000.000")
            }
        },
        {
            .record_index = 2U,
            .deleted = false,
            .values = {
                value("OBJTYPE", "8"),
                value("EXPR", "customer.amount"),
                value("OFFSET", "1", 612U),
                value("HPOS", "100.000"),
                value("VPOS", "200.000"),
                value("WIDTH", "800.000"),
                value("HEIGHT", "600.000")
            }
        },
        {
            .record_index = 3U,
            .deleted = true,
            .values = {
                value("OBJTYPE", "8"),
                value("EXPR", "customer.deleted_amount"),
                value("OFFSET", "2", 613U),
                value("HPOS", "900.000"),
                value("VPOS", "200.000"),
                value("WIDTH", "800.000"),
                value("HEIGHT", "600.000")
            }
        }
    };

    const auto layout = copperfin::studio::build_report_layout(document);
    expect(layout.available && layout.sections.size() == 1U && layout.sections[0].objects.size() == 1U,
        "#4523: report-expression objects should remain in their containing section");
    if (layout.sections.size() == 1U && layout.sections[0].objects.size() == 1U) {
        const auto& live = layout.sections[0].objects[0];
        const auto live_highlight = std::find_if(
            live.highlights.begin(),
            live.highlights.end(),
            [](const auto& highlight) {
                return highlight.name == "OFFSET";
            });
        expect(live_highlight != live.highlights.end(),
            "#4523: live report-expression highlights should include OFFSET");
        if (live_highlight != live.highlights.end()) {
            expect(live_highlight->value == "1" &&
                   live_highlight->field_index == 2U &&
                   live_highlight->memo_block_number == 612U,
                "#4523: live OFFSET should preserve value and provenance");
        }
    }

    const auto deleted = std::find_if(
        layout.deleted_objects.begin(),
        layout.deleted_objects.end(),
        [](const auto& object) {
            return object.record_index == 3U;
        });
    expect(deleted != layout.deleted_objects.end(),
        "#4523: deleted report-expression objects should remain available for inspection");
    if (deleted != layout.deleted_objects.end()) {
        const auto deleted_highlight = std::find_if(
            deleted->highlights.begin(),
            deleted->highlights.end(),
            [](const auto& highlight) {
                return highlight.name == "OFFSET";
            });
        expect(deleted_highlight != deleted->highlights.end() &&
               deleted_highlight->value == "2" &&
               deleted_highlight->field_index == 2U &&
               deleted_highlight->memo_block_number == 613U,
            "#4523: deleted OFFSET should preserve value and provenance");
    }
}

}  // namespace cf_test_report_layout
