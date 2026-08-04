// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.

#include "test_report_layout_support.h"

namespace cf_test_report_layout {

void test_report_object_tooltip_provenance() {
    copperfin::studio::StudioDocumentModel document;
    document.display_name = "tooltip.frx";
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
                value("OBJTYPE", "5"),
                value("TAG2", "label tip", 700U),
                value("HPOS", "100.000"),
                value("VPOS", "200.000"),
                value("WIDTH", "800.000"),
                value("HEIGHT", "400.000")
            }
        },
        {
            .record_index = 3U,
            .deleted = false,
            .values = {
                value("OBJTYPE", "6"),
                value("TAG2", "line tip", 701U),
                value("HPOS", "900.000"),
                value("VPOS", "200.000"),
                value("WIDTH", "800.000"),
                value("HEIGHT", "400.000")
            }
        },
        {
            .record_index = 4U,
            .deleted = true,
            .values = {
                value("OBJTYPE", "7"),
                value("TAG2", "deleted shape tip", 702U),
                value("HPOS", "1300.000"),
                value("VPOS", "200.000"),
                value("WIDTH", "800.000"),
                value("HEIGHT", "400.000")
            }
        },
        {
            .record_index = 5U,
            .deleted = false,
            .values = {
                value("OBJTYPE", "8"),
                value("EXPR", "customer.amount"),
                value("TAG2", "expression tip", 703U),
                value("HPOS", "1700.000"),
                value("VPOS", "200.000"),
                value("WIDTH", "800.000"),
                value("HEIGHT", "400.000")
            }
        },
        {
            .record_index = 6U,
            .deleted = false,
            .values = {
                value("OBJTYPE", "17"),
                value("TAG2", "image tip", 704U),
                value("HPOS", "2500.000"),
                value("VPOS", "200.000"),
                value("WIDTH", "800.000"),
                value("HEIGHT", "400.000")
            }
        },
        {
            .record_index = 7U,
            .deleted = false,
            .values = {
                value("OBJTYPE", "18"),
                value("TAG2", "variable tip", 705U),
                value("HPOS", "3300.000"),
                value("VPOS", "200.000"),
                value("WIDTH", "800.000"),
                value("HEIGHT", "400.000")
            }
        }
    };

    const auto layout = copperfin::studio::build_report_layout(document);
    expect(layout.available && layout.sections.size() == 1U && layout.sections[0].objects.size() == 5U,
        "#4530: report controls should remain in their containing section");

    const auto find_live = [&](std::size_t record_index) {
        return std::find_if(
            layout.sections[0].objects.begin(),
            layout.sections[0].objects.end(),
            [record_index](const auto& object) {
                return object.record_index == record_index;
            });
    };
    const auto expect_tooltip = [&](std::size_t record_index, const std::string& expected, std::size_t field_index, std::uint32_t memo_block) {
        if (layout.sections.empty()) {
            return;
        }
        const auto object = find_live(record_index);
        expect(object != layout.sections[0].objects.end(),
            "#4530: eligible live report control should remain selectable");
        if (object != layout.sections[0].objects.end()) {
            const auto tooltip = std::find_if(
                object->highlights.begin(),
                object->highlights.end(),
                [](const auto& highlight) {
                    return highlight.name == "TAG2";
                });
            expect(tooltip != object->highlights.end() &&
                   tooltip->value == expected &&
                   tooltip->field_index == field_index &&
                   tooltip->memo_block_number == memo_block,
                "#4530: live TAG2 tooltip should preserve value and memo provenance");
        }
    };

    if (!layout.sections.empty()) {
        expect_tooltip(2U, "label tip", 1U, 700U);
        expect_tooltip(3U, "line tip", 1U, 701U);
        expect_tooltip(5U, "expression tip", 2U, 703U);
        expect_tooltip(6U, "image tip", 1U, 704U);

        const auto variable = find_live(7U);
        expect(variable != layout.sections[0].objects.end(),
            "#4530: non-tooltip variable object should remain selectable");
        if (variable != layout.sections[0].objects.end()) {
            expect(std::none_of(
                       variable->highlights.begin(),
                       variable->highlights.end(),
                       [](const auto& highlight) {
                           return highlight.name == "TAG2";
                       }),
                "#4530: variable TAG2 should not be mislabeled as a report-control tooltip");
        }
    }

    const auto deleted = std::find_if(
        layout.deleted_objects.begin(),
        layout.deleted_objects.end(),
        [](const auto& object) {
            return object.record_index == 4U;
        });
    expect(deleted != layout.deleted_objects.end(),
        "#4530: deleted report controls should remain available for inspection");
    if (deleted != layout.deleted_objects.end()) {
        const auto tooltip = std::find_if(
            deleted->highlights.begin(),
            deleted->highlights.end(),
            [](const auto& highlight) {
                return highlight.name == "TAG2";
            });
        expect(tooltip != deleted->highlights.end() &&
               tooltip->value == "deleted shape tip" &&
               tooltip->field_index == 1U &&
               tooltip->memo_block_number == 702U,
            "#4530: deleted TAG2 tooltip should preserve value and memo provenance");
    }
}

}  // namespace cf_test_report_layout
