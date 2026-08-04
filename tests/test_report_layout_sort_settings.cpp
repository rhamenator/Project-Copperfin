// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.

#include "copperfin/studio/report_layout.h"

#include <algorithm>
#include <cstdint>
#include <iostream>
#include <string>
#include <utility>
#include <vector>

namespace {

int failures = 0;

void expect(bool condition, const std::string& message) {
    if (!condition) {
        std::cerr << "FAIL: " << message << "\n";
        ++failures;
    }
}

copperfin::vfp::DbfRecordValue value(
    std::string name,
    std::string display_value,
    std::uint32_t memo_block_number = 0U) {
    return {
        .field_name = std::move(name),
        .field_type = 'C',
        .is_null = false,
        .display_value = std::move(display_value),
        .memo_block_number = memo_block_number
    };
}

void test_build_report_layout_includes_direct_tag_settings() {
    copperfin::studio::StudioDocumentModel document;
    document.display_name = "tagged.frx";
    document.kind = copperfin::studio::StudioAssetKind::report;
    document.table_preview_available = true;

    document.table_preview.records = {
        {
            .record_index = 0U,
            .deleted = false,
            .values = {
                value("OBJTYPE", "1"),
                value("OBJCODE", "53"),
                value("EXPR", "ORIENTATION=1\r\nPAPERSIZE=9", 44U),
                value("TOPMARGIN", "10"),
                value("TAG", "customer.country")
            }
        }
    };

    const auto layout = copperfin::studio::build_report_layout(document);
    expect(layout.available, "#2908: report layout should stay available for TAG-backed report settings");
    expect(layout.settings.size() == 4U,
           "#2908: direct TAG settings should be counted alongside memo and direct report settings");

    const auto tag = std::find_if(layout.settings.begin(), layout.settings.end(), [](const auto& setting) {
        return setting.name == "TAG";
    });
    expect(tag != layout.settings.end(), "#2908: direct TAG settings should surface in live report settings");
    if (tag != layout.settings.end()) {
        expect(tag->record_index == 0U, "#2908: direct TAG settings should retain root record provenance");
        expect(tag->field_index == 4U, "#2908: direct TAG settings should retain DBF field provenance");
        expect(tag->source_line_index == copperfin::studio::StudioReportMissingLineIndex,
               "#2908: direct TAG settings should not masquerade as memo-line settings");
        expect(tag->memo_block_number == 0U,
               "#2908: direct TAG settings should expose memo block zero for non-memo-backed roots");
        expect(tag->value == "customer.country",
               "#2908: direct TAG settings should preserve sort-expression values");
    }
}

void test_build_report_layout_includes_deleted_direct_tag_settings() {
    copperfin::studio::StudioDocumentModel document;
    document.display_name = "deleted-tagged.frx";
    document.kind = copperfin::studio::StudioAssetKind::report;
    document.table_preview_available = true;

    document.table_preview.records = {
        {
            .record_index = 0U,
            .deleted = true,
            .values = {
                value("OBJTYPE", "1"),
                value("OBJCODE", "53"),
                value("EXPR", "ORIENTATION=1", 45U),
                value("TAG", "customer.region", 12U)
            }
        }
    };

    const auto layout = copperfin::studio::build_report_layout(document);
    expect(layout.available, "#2908: report layout should stay available for deleted TAG-backed root settings");
    expect(layout.settings.empty(),
           "#2908: deleted TAG settings should not leak into live report settings");
    expect(layout.deleted_settings.size() == 2U,
           "#2908: deleted TAG settings should be counted with deleted report settings");

    const auto tag = std::find_if(layout.deleted_settings.begin(), layout.deleted_settings.end(), [](const auto& setting) {
        return setting.name == "TAG";
    });
    expect(tag != layout.deleted_settings.end(), "#2908: deleted TAG settings should surface in deleted report settings");
    if (tag != layout.deleted_settings.end()) {
        expect(tag->record_index == 0U, "#2908: deleted TAG settings should retain root record provenance");
        expect(tag->field_index == 3U, "#2908: deleted TAG settings should retain DBF field provenance");
        expect(tag->source_line_index == copperfin::studio::StudioReportMissingLineIndex,
               "#2908: deleted TAG settings should not masquerade as memo-line settings");
        expect(tag->memo_block_number == 12U,
               "#2908: deleted TAG settings should preserve memo block provenance");
        expect(tag->value == "customer.region",
               "#2908: deleted TAG settings should preserve deleted sort-expression values");
    }
}

void test_build_report_layout_ignores_blank_direct_tag_settings() {
    copperfin::studio::StudioDocumentModel document;
    document.display_name = "blank-tagged.frx";
    document.kind = copperfin::studio::StudioAssetKind::report;
    document.table_preview_available = true;

    document.table_preview.records = {
        {
            .record_index = 0U,
            .deleted = false,
            .values = {
                value("OBJTYPE", "1"),
                value("OBJCODE", "53"),
                value("EXPR", "ORIENTATION=1", 46U),
                value("TAG", "   ")
            }
        }
    };

    const auto layout = copperfin::studio::build_report_layout(document);
    const auto tag = std::find_if(layout.settings.begin(), layout.settings.end(), [](const auto& setting) {
        return setting.name == "TAG";
    });
    expect(tag == layout.settings.end(),
           "#2908: blank TAG settings should stay absent like other blank report settings");
}

}  // namespace

int main() {
    test_build_report_layout_includes_direct_tag_settings();
    test_build_report_layout_includes_deleted_direct_tag_settings();
    test_build_report_layout_ignores_blank_direct_tag_settings();

    if (failures != 0) {
        std::cerr << failures << " test(s) failed.\n";
        return 1;
    }

    std::cout << "All tests passed.\n";
    return 0;
}
