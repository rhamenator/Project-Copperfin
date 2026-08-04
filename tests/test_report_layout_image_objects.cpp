// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.

#include "test_report_layout_support.h"

namespace cf_test_report_layout {

void test_report_image_picture_provenance() {
    copperfin::studio::StudioDocumentModel document;
    document.display_name = "image.frx";
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
                value("OBJTYPE", "17"),
                value("PICTURE", "images\\logo.bmp", 412U),
                value("HPOS", "100.000"),
                value("VPOS", "200.000"),
                value("WIDTH", "800.000"),
                value("HEIGHT", "600.000")
            }
        }
    };

    const auto layout = copperfin::studio::build_report_layout(document);
    expect(layout.available && layout.sections.size() == 1U && layout.sections[0].objects.size() == 1U,
        "#4521: image report objects should remain in their containing section");
    if (layout.sections.size() == 1U && layout.sections[0].objects.size() == 1U) {
        const auto& image = layout.sections[0].objects[0];
        expect(image.picture == "images\\logo.bmp",
            "#4521: image report objects should preserve PICTURE source values");
        expect(image.picture_field_index == 1U && image.picture_memo_block_number == 412U,
            "#4521: image PICTURE should preserve field and memo provenance");
        const auto picture_highlight = std::find_if(
            image.highlights.begin(),
            image.highlights.end(),
            [](const auto& highlight) {
                return highlight.name == "PICTURE";
            });
        expect(picture_highlight != image.highlights.end(),
            "#4521: image report highlights should include PICTURE");
        if (picture_highlight != image.highlights.end()) {
            expect(picture_highlight->value == "images\\logo.bmp" &&
                   picture_highlight->field_index == 1U &&
                   picture_highlight->memo_block_number == 412U,
                "#4521: image PICTURE highlights should preserve value and provenance");
        }
    }
}

}  // namespace cf_test_report_layout
