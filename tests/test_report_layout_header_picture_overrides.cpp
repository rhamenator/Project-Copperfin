// Copyright © 2026 Richard M. Hamilton. All rights reserved.
// Licensed under the Project Copperfin Source-Available License or
// Commercial License. See LICENSE.md in the repository root.

#include "test_report_layout_support.h"

namespace cf_test_report_layout {

void test_report_header_picture_overrides_provenance() {
    copperfin::studio::StudioDocumentModel document;
    document.display_name = "header-picture-overrides.frx";
    document.kind = copperfin::studio::StudioAssetKind::report;
    document.table_preview_available = true;
    document.table_preview.records = {
        {
            .record_index = 0U,
            .deleted = false,
            .values = {
                value("OBJTYPE", "1"),
                value("EXPR", "COLOR=2\r\nYRESOLUTION=300\r\nUNSUPPORTED=expr", 707U),
                value("PICTURE", "COLOR=1\r\nCOPIES=3\r\nUNSUPPORTED=picture", 708U),
                value("COLOR", "9")
            }
        }
    };

    const auto layout = copperfin::studio::build_report_layout(document);
    expect(layout.available, "#4539: header printer override fixture should produce a report layout snapshot");

    const auto find_setting = [](const auto& settings, std::string_view name) {
        return std::find_if(
            settings.begin(),
            settings.end(),
            [name](const auto& setting) { return setting.name == name; });
    };

    const auto color = find_setting(layout.settings, "COLOR");
    expect(color != layout.settings.end() && color->value == "1" &&
           color->field_index == 2U && color->source_line_index == 0U &&
           color->memo_block_number == 708U,
        "#4539: header PICTURE should override EXPR COLOR with PICTURE provenance");

    const auto y_resolution = find_setting(layout.settings, "YRESOLUTION");
    expect(y_resolution != layout.settings.end() && y_resolution->value == "300" &&
           y_resolution->field_index == 1U && y_resolution->source_line_index == 1U &&
           y_resolution->memo_block_number == 707U,
        "#4539: non-overridden EXPR printer settings should retain their provenance");

    const auto copies = find_setting(layout.settings, "COPIES");
    expect(copies != layout.settings.end() && copies->value == "3" &&
           copies->field_index == 2U && copies->source_line_index == 1U &&
           copies->memo_block_number == 708U,
        "#4539: unique header PICTURE printer settings should be exposed with provenance");

    const auto unsupported = find_setting(layout.settings, "UNSUPPORTED");
    expect(unsupported != layout.settings.end() && unsupported->value == "picture" &&
           unsupported->source_line_index == 2U && unsupported->memo_block_number == 708U,
        "#4539: PICTURE override precedence should preserve unsupported duplicate content");
    expect(layout.settings.size() == 4U,
        "#4539: PICTURE overrides should replace duplicate settings without duplicating keys");
}

}  // namespace cf_test_report_layout
