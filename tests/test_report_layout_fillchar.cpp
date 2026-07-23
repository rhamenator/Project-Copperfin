// Copyright © 2026 Richard M. Hamilton. All rights reserved.
// Licensed under the Project Copperfin Source-Available License or
// Commercial License. See LICENSE.md in the repository root.

#include "test_report_layout_support.h"

namespace cf_test_report_layout {

void test_report_expression_fillchar_provenance() {
    copperfin::studio::StudioDocumentModel document;
    document.display_name = "fillchar.frx";
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
                value("FILLCHAR", "N", 614U),
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
                value("EXPR", "customer.deleted_date"),
                value("FILLCHAR", "D", 615U),
                value("HPOS", "900.000"),
                value("VPOS", "200.000"),
                value("WIDTH", "800.000"),
                value("HEIGHT", "600.000")
            }
        },
        {
            .record_index = 4U,
            .deleted = false,
            .values = {
                value("OBJTYPE", "8"),
                value("EXPR", "customer.name"),
                value("FILLCHAR", "", 616U),
                value("HPOS", "1700.000"),
                value("VPOS", "200.000"),
                value("WIDTH", "800.000"),
                value("HEIGHT", "600.000")
            }
        },
        {
            .record_index = 5U,
            .deleted = false,
            .values = {
                value("OBJTYPE", "5"),
                value("EXPR", "customer.label"),
                value("FILLCHAR", "C", 617U),
                value("HPOS", "2500.000"),
                value("VPOS", "200.000"),
                value("WIDTH", "800.000"),
                value("HEIGHT", "600.000")
            }
        }
    };

    const auto layout = copperfin::studio::build_report_layout(document);
    expect(layout.available && layout.sections.size() == 1U && layout.sections[0].objects.size() == 3U,
        "#4524: report-expression and label objects should remain in their containing section");
    if (layout.sections.size() == 1U && layout.sections[0].objects.size() == 3U) {
        const auto& live = layout.sections[0].objects[0];
        const auto live_highlight = std::find_if(
            live.highlights.begin(),
            live.highlights.end(),
            [](const auto& highlight) {
                return highlight.name == "FILLCHAR";
            });
        expect(live_highlight != live.highlights.end(),
            "#4524: live report-expression highlights should include FILLCHAR");
        if (live_highlight != live.highlights.end()) {
            expect(live_highlight->value == "N" &&
                   live_highlight->field_index == 2U &&
                   live_highlight->memo_block_number == 614U,
                "#4524: live FILLCHAR should preserve value and provenance");
        }

        const auto& blank = layout.sections[0].objects[1];
        expect(std::none_of(
                   blank.highlights.begin(),
                   blank.highlights.end(),
                   [](const auto& highlight) {
                       return highlight.name == "FILLCHAR";
                   }),
            "#4524: blank FILLCHAR should remain an observable blank without a fabricated highlight");

        const auto& label = layout.sections[0].objects[2];
        expect(std::none_of(
                   label.highlights.begin(),
                   label.highlights.end(),
                   [](const auto& highlight) {
                       return highlight.name == "FILLCHAR";
                   }),
            "#4524: label FILLCHAR should not be treated as an expression data-type contract");
    }

    const auto deleted = std::find_if(
        layout.deleted_objects.begin(),
        layout.deleted_objects.end(),
        [](const auto& object) {
            return object.record_index == 3U;
        });
    expect(deleted != layout.deleted_objects.end(),
        "#4524: deleted report-expression objects should remain available for inspection");
    if (deleted != layout.deleted_objects.end()) {
        const auto deleted_highlight = std::find_if(
            deleted->highlights.begin(),
            deleted->highlights.end(),
            [](const auto& highlight) {
                return highlight.name == "FILLCHAR";
            });
        expect(deleted_highlight != deleted->highlights.end() &&
               deleted_highlight->value == "D" &&
               deleted_highlight->field_index == 2U &&
               deleted_highlight->memo_block_number == 615U,
            "#4524: deleted FILLCHAR should preserve value and provenance");
    }
}

}  // namespace cf_test_report_layout
